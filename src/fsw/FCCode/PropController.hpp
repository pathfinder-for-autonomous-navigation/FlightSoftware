#pragma once
#include <fsw/FCCode/TimedControlTask.hpp>
#include <fsw/FCCode/Drivers/PropulsionSystem.hpp>
#include <fsw/FCCode/prop_state_t.enum>
#include <vector>

/**
 * Implementation Info:
 * - millisecond to control cycle count conversions take the floor operator - change this by changing the constexprs
 * - There is no going from Pressurizing directly to Firing
 *
 */

// Forward declaration of PropState classes
class PropState;
class PropState_Disabled;
class PropState_Idle;
class PropState_AwaitPressurizing;
class PropState_Pressurizing;
class PropState_AwaitFiring;
class PropState_Firing;
class PropState_Venting;
class PropState_HandlingFault;

class PropController : public TimedControlTask<void> {
public:
    PropController(StateFieldRegistry &registry, unsigned int offset);

    // ------------------------------------------------------------------------
    // Public Interface
    // ------------------------------------------------------------------------
    void execute() override;
    bool validate_schedule();

    WritableStateField<unsigned int>prop_state_f;

    WritableStateField<unsigned int>fire_cycle_f;
    WritableStateField<unsigned int>sched_valve1_f;
    WritableStateField<unsigned int>sched_valve2_f;
    WritableStateField<unsigned int>sched_valve3_f;
    WritableStateField<unsigned int>sched_valve4_f;

    // ------------------------------------------------------------------------
    // Helper Functions
    // ------------------------------------------------------------------------

    // Return true if Tank2 is at threshold pressure
    static bool is_at_threshold_pressure();

    inline bool check_current_state(prop_state_t expected) const {
        return expected == static_cast<prop_state_t>(prop_state_f.get());
    }

    bool is_valid_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4,
                           unsigned int ctrl_cycles_from_now);

    // Copies the schedule in the writable state fields into Tank2's schedule
    // Precond: schedule should be valid before calling this
    inline void write_tank2_schedule() {
        PropulsionSystem.set_schedule(sched_valve1_f.get(), sched_valve2_f.get(), sched_valve3_f.get(),
                                      sched_valve4_f.get());
    }

    // Return True if we are allowed to enter the desired_state
    bool can_enter_state(prop_state_t desired_state) const;


    // ------------------------------------------------------------------------
    // Constants
    // ------------------------------------------------------------------------
    static constexpr unsigned int max_pressurizing_cycles = 20;
    static constexpr float threshold_firing_pressure = 25;

private:
    // Return the PropState associated with the given prop_state_t
    PropState &get_state(prop_state_t) const;

    // ------------------------------------------------------------------------
    // Static Components
    // ------------------------------------------------------------------------
    static PropState_Disabled state_disabled;
    static PropState_Idle state_idle;
    static PropState_AwaitPressurizing state_await_pressurizing;
    static PropState_Pressurizing state_pressurizing;
    static PropState_AwaitFiring state_await_firing;
    static PropState_Firing state_firing;
    // static PropState_Venting state_venting;
    // static PropState_HandlingFault state_handling_fault;
};

// ------------------------------------------------------------------------
// CountdownTimer
// ------------------------------------------------------------------------

// This class is like a countdown timer on a bomb
// It uses units of control cycles
class CountdownTimer {
public:
    bool is_timer_zero() const;
    // Sets the timer (does not check whether the timer is free)
    void set_timer_cc(size_t num_control_cycles);

    void reset_timer();

    void tick();

private:
    // Number of cycles until this timer is free
    size_t cycles_left = 0;
};

// ------------------------------------------------------------------------
// Propulsion States
// ------------------------------------------------------------------------

// Abstract class that represents a Propulsion System State
class PropState {
public:
    explicit PropState(prop_state_t my_state)
            : this_state(my_state) {}

    // We call this function when we are about to enter this state. 
    // It checks the preconditions for entering the state
    virtual bool can_enter() const = 0;

    // Actually enter the state and runs initialization routines for the state
    virtual void enter() = 0;

    // We call this when we are in this state and want to figure out whether
    //      we should transition to a different state. This function evaluates
    //      the current state of the PropulsionSystem and returns the next state
    //      If the returned state is the same as this current state (this_state)
    //      that means we are not changing states this cycle. 
    virtual prop_state_t evaluate() = 0;

protected:
    const prop_state_t this_state;

    // All instances of PropState will hold a reference to PropController in order
    // to call functions in PropController
    static PropController* controller;
    // The only purpose of declaring PropController a friend class is to allow it
    //      to set controller to itself
    friend class PropController;
};

class PropState_Disabled : public PropState {
public:
    PropState_Disabled() : PropState(prop_state_t::disabled) {}

    bool can_enter() const override;

    void enter() override;

    prop_state_t evaluate() override;
};

class PropState_Idle : public PropState {
public:
    PropState_Idle() : PropState(prop_state_t::idle) {}

    bool can_enter() const override;

    void enter() override;

    prop_state_t evaluate() override;

    // Start pressurizing when we are within this amount of control cycles of the time
    //      at which we want to fire
    static constexpr unsigned int num_cycles_within_firing_to_pressurize = 50;

};

// This is the state where we've received a (valid) request to fire.
// NO ONE may change firing parameters once this state is entered. However, this state can be cancelled to go back to Idle
class PropState_AwaitPressurizing : public PropState {
public:
    PropState_AwaitPressurizing() : PropState(prop_state_t::await_pressurizing) {}

    bool can_enter() const override;

    void enter() override;

    prop_state_t evaluate() override;

private:

    // Copies the values in sched_valve1_f, sched_valve2_f, etc. into Tank2's schedule
    void write_tank2_schedule();
};

// A pressurizing cycle consists of a "firing" period and a "cooling period". 
//      The firing period is a 1 second duration in which an intertank valve is opened
//      The cooling period is a 10 second duration in which no valve may be opened
// If we have executed 20 consecutive pressurizing cycles and have not yet reached
//      threshold pressure, then this is a fault
// [ cc1 ][ cc2 ][ cc3 ][ cc4 ][ cc5 ][ cc6 ][ cc7 ] <-- control cycles
// [    pressurize cycle (1s) ][    cool off time (10s)                    ...]
class PropState_Pressurizing : public PropState {
public:
    PropState_Pressurizing() : PropState(prop_state_t::pressurizing), pressurizing_cycle_count(0) {}

    bool can_enter() const override;

    void enter() override;

    prop_state_t evaluate() override;

    static constexpr unsigned int ctrl_cycles_per_firing_period = 1000 / PAN::control_cycle_time_ms;
    static constexpr unsigned int ctrl_cycles_per_cooling_period = 10 * 1000 / PAN::control_cycle_time_ms;

    // Number of control cycles in 1 pressurizing cycle
    static constexpr unsigned int ctrl_cycles_per_pressurizing_cycle
            = ctrl_cycles_per_firing_period + ctrl_cycles_per_cooling_period;

    // Number of control cycles needed to pressurize Tank2
    //  20 firings + 19 coolings (because of the fence rule)
    static unsigned int num_cycles_needed();

    // Return true if we should start pressurizing right now.
    static bool is_time_to_pressurize();

private:
    // Called when Tank1 valve is currently open
    prop_state_t handle_valve_is_open();
    // Called when Tank1 valve is currently closed
    prop_state_t handle_valve_is_close();
    // Called when we have failed to reach threshold_pressure after maximum consecutive pressurizing cycles
    prop_state_t handle_pressurize_failed();
    // Starts another pressurization cycle
    void start_pressurize_cycle();

    // Returns true if we should use the backup valve for pressurizing
    static bool should_use_backup();

    // 1 if we are using the backup valve and 0 otherwise
    bool valve_num = false;
    // Timer to time the 1s firing period and the 10s cooling period
    CountdownTimer countdown;
    // Number of pressurizing cycles since we last entered this state
    //  If this number is >= 20, then signal pressurize failure fault
    unsigned int pressurizing_cycle_count;
    friend class PropController;
};

class PropState_Firing : public PropState {
public:
    PropState_Firing() : PropState(prop_state_t::firing) {}

    bool can_enter() const override;

    void enter() override;

    prop_state_t evaluate() override;

private:
    // Returns true if the entire schedule is all zeros
    bool is_schedule_empty() const;
};

class PropState_AwaitFiring : public PropState {
public:
    PropState_AwaitFiring() : PropState(prop_state_t::await_firing) {}

    bool can_enter() const override;

    void enter() override;

    prop_state_t evaluate() override;

private:
    // Return true if it is time to fire
    bool is_time_to_fire() const;
};

class PropState_Venting : public PropState {
    // TODO: not yet implemented nor used
public:
    PropState_Venting() : PropState(prop_state_t::venting) {}

    bool can_enter() const override;

    void enter() override;

    prop_state_t evaluate() override;
};

class PropState_HandlingFault : public PropState {
    // TODO: not yet implemented nor use
public:
    PropState_HandlingFault() : PropState(prop_state_t::handling_fault) {}

    bool can_enter() const override;

    void enter() override;

    prop_state_t evaluate() override;
};