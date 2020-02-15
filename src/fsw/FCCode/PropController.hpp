#pragma once
#include <fsw/FCCode/TimedControlTask.hpp>
#include <fsw/FCCode/Drivers/PropulsionSystem.hpp>
#include <fsw/FCCode/prop_state_t.enum>
#include <vector>
class PropState;
class PropState_Disabled;
class PropState_Idle;
class PropState_Pressurizing;
class PropState_AwaitFiring;
class PropState_Firing;
class PropState_Venting;
class PropState_HandlingFault;

class PropController : public TimedControlTask<void> {

public:
    PropController(StateFieldRegistry& registry, unsigned int offset);

    // ------------------------------------------------------------------------
    // Public Interface
    // ------------------------------------------------------------------------

    void execute() override;

    void set_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4, unsigned int ctrl_cycles_from_now);

    bool validate_schedule();

    // Dangerous: prop_state_f can be changed at anytime by outside components
    WritableStateField<unsigned int>prop_state_f;

    // the cycle at which we should fire
    WritableStateField<unsigned int>fire_cycle_f;
    WritableStateField<unsigned int>sched_valve1_f;
    WritableStateField<unsigned int>sched_valve2_f;
    WritableStateField<unsigned int>sched_valve3_f;
    WritableStateField<unsigned int>sched_valve4_f;

    bool is_valid_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4, unsigned int ctrl_cycles_from_now);
    
    // ------------------------------------------------------------------------
    // State Functions
    // ------------------------------------------------------------------------
    
    // Return the PropState associated with the given prop_state_t
    PropState& get_state(prop_state_t);


    inline void change_state(prop_state_t new_state)
    {
        prop_state_f.set(static_cast<unsigned int>(new_state)); 
    }

    // Return true if Tank2 is at threshold pressure
    inline static bool is_at_threshold_pressure()
    {
#ifdef PROP_TEST
        return true;
#else
        return Tank2.get_pressure() >= threshold_firing_pressure;
#endif
    }

    inline bool check_current_state(prop_state_t expected)
    {
        return expected == static_cast<prop_state_t>(prop_state_f.get());
    }

    // ------------------------------------------------------------------------
    // Constants
    // ------------------------------------------------------------------------

    static constexpr unsigned int max_pressurizing_cycles = 20;
    static constexpr float threshold_firing_pressure = 25;

private:
    // ------------------------------------------------------------------------
    // Static Components
    // ------------------------------------------------------------------------
    static PropState_Disabled state_disabled;
    static PropState_Idle state_idle;
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
    CountdownTimer();
    bool is_timer_zero() const;
    // Sets the timer (does not check whether the timer)
    void set_timer_cc(size_t num_control_cycles);
    // Converts num_ms into control_cycles (and rounds down)
    void set_timer_ms(size_t num_ms);
    void reset_timer();
private:
    // Number of cycles until this timer is free
    size_t cycles_left = 0;

    // Called by PropController to advance each timer in tick_list by one control cycle
    static void tick(); 
    // All instantiated timers are automatically added to this tick_list
    static std::vector<CountdownTimer*> tick_list;
    friend class PropController;
};

// ------------------------------------------------------------------------
// Propulsion States
// ------------------------------------------------------------------------

// abstract class that represents a Propulsion System State
class PropState
{
public:
    PropState(prop_state_t my_state) 
    : this_state(my_state) {}
    virtual bool can_enter() = 0;
    virtual prop_state_t evaluate() = 0;

protected:
    prop_state_t this_state;
    static PropController* controller;
    friend class PropController;
};

class PropState_Disabled : public PropState
{
public:
    PropState_Disabled() : PropState(prop_state_t::disabled) {}
    bool can_enter() override;
    prop_state_t evaluate() override;
};

class PropState_Idle : public PropState
{
public:
    PropState_Idle() : PropState(prop_state_t::idle) {}
    bool can_enter() override;
    prop_state_t evaluate() override;
    // start pressurizing when we are within this amount of control cycles of the time
    // at which we want to fire
    static constexpr unsigned int num_cycles_within_firing_to_pressurize = 50;
private:
    bool is_time_to_pressurize() const;
};

// A pressurizing cycle is a 1 second duration in which an intertank valve is opened
// A single pressurizing cycle will span multiple control cycles
// If we have executed 20 consecutive pressurizing cycles and have not yet reached
// threshold pressure, then this is a fault

// [ cc1 ][ cc2 ][ cc3 ][ cc4 ][ cc5 ][ cc6 ][ cc7 ] <-- control cycles
// [    pressurize cycle (1s) ][    cool off time (10s)                    ...]
class PropState_Pressurizing : public PropState
{
public:
    PropState_Pressurizing() : PropState(prop_state_t::pressurizing) {}
    bool can_enter() override;
    prop_state_t evaluate() override;

    // Return true if we have enough time to pressurize Tank2
    static bool can_pressurize_in_time();
    // Number of control cycles needed to pressurize Tank2
    static unsigned int num_cycles_needed();
    static constexpr unsigned int firing_duration_ms = 1000;
    static constexpr unsigned int cooling_duration_ms = 10*1000;
    static constexpr unsigned int ctrl_cycles_per_pressurizing_cycle = 
    (firing_duration_ms + cooling_duration_ms)/PAN::control_cycle_time_ms;
private:
    // Called when Tank1 valve is currently open
    void handle_valve_is_open();
    // Called when Tank1 valve is currently closed
    void handle_valve_is_close();
    // Called when we have failed to reach threshod_pressure after maximum consecutive pressurizing cycles
    void handle_pressurize_failed();

    // Starts another pressurization cycle
    void start_pressurize_cycle();
    // Returns true if we should use the backup valve for pressurizing
    static bool should_use_backup();
    // 1 if we are using the backup valve and 0 otherwise
    bool valve_num = 0;
    // Timer to time the 1s firing period and the 10s cooling period
    CountdownTimer countdown;
    // Number of pressurizing cycles since we last entered this state
    // If this number is >= 20, then signal pressurize failure fault
    unsigned int pressurizing_cycle_count;
};


class PropState_Firing : public PropState
{
public:
    PropState_Firing() : PropState(prop_state_t::firing) {}
    bool can_enter() override;
    prop_state_t evaluate() override;

private:
    // Returns true if the entire schedule is all zeros
    bool is_schedule_empty();
};



class PropState_AwaitFiring : public PropState
{
public:
    PropState_AwaitFiring() : PropState(prop_state_t::await_firing) {}
    bool can_enter() override;
    prop_state_t evaluate() override;

private:
    // return true if it is time to fire
    bool is_time_to_fire();
};

class PropState_Venting : public PropState
{
public:
    PropState_Venting() : PropState(prop_state_t::venting) {}
    bool can_enter() override;
    prop_state_t evaluate() override;
};

class PropState_HandlingFault : public PropState
{
public:
    PropState_HandlingFault() : PropState(prop_state_t::handling_fault) {}
    bool can_enter() override;
    prop_state_t evaluate() override;
};