#pragma once

#include <fsw/FCCode/TimedControlTask.hpp>
#include <fsw/FCCode/Drivers/PropulsionSystem.hpp>
#include <fsw/FCCode/prop_state_t.enum>
#include <common/Fault.hpp>
/**
 * Implementation Info:
 * - millisecond to control cycle count conversions take the floor operator - change this by changing the constexprs
 * - There is no going from Pressurizing directly to Firing
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
class PropState_Manual;
class PropController : public TimedControlTask<void>
{
public:
    PropController(StateFieldRegistry &registry, unsigned int offset);

    // ------------------------------------------------------------------------
    // Input Fields
    // ------------------------------------------------------------------------

    WritableStateField<unsigned int> prop_state_f;

    WritableStateField<unsigned int> cycles_until_firing;
    WritableStateField<unsigned int> sched_valve1_f;
    WritableStateField<unsigned int> sched_valve2_f;
    WritableStateField<unsigned int> sched_valve3_f;
    WritableStateField<unsigned int> sched_valve4_f;

    WritableStateField<unsigned int> sched_intertank1_f;
    WritableStateField<unsigned int> sched_intertank2_f;

    // ------------------------------------------------------------------------
    // Ground-Modifiable Parameters
    // ------------------------------------------------------------------------

    WritableStateField<unsigned int> max_pressurizing_cycles;
    WritableStateField<float> threshold_firing_pressure;
    WritableStateField<unsigned int> ctrl_cycles_per_filling_period;
    WritableStateField<unsigned int> ctrl_cycles_per_cooling_period;
    WritableStateField<unsigned int> tank1_valve;

    // ------------------------------------------------------------------------
    // Output Fields
    // ------------------------------------------------------------------------

    ReadableStateField<float> tank2_pressure_f;
    ReadableStateField<float> tank2_temp_f;
    ReadableStateField<float> tank1_temp_f;

    Fault pressurize_fail_fault_f; // underpressurized
    Fault overpressure_fault_f;    // overpressurized
    Fault tank2_temp_high_fault_f;
    Fault tank1_temp_high_fault_f;

    // ------------------------------------------------------------------------
    // Public Interface
    // ------------------------------------------------------------------------

    // Reads PropulsionSystem sensor values and saves them in state fields.
    // If there are hardware faults, signal those faults.
    // If any fault is faulted, then set our current state to handling_fault.
    // Evaluate the current state to get the next state
    // If the next state differs from the current state, update our current state and then call the state's entry() method
    void execute() override;

    // Checks the sensor values of Tank1 and Tank2.
    // Signals the fault and returns true if any hardware fault is detected
    void check_faults();

    bool validate_schedule();

    // Minimum of cycles needed to prepare for firing time
    unsigned int min_cycles_needed() const;

    // Return true if Tank2 is at threshold pressure
    bool is_at_threshold_pressure();

    inline bool is_tank2_overpressured() const
    {
        return tank2_pressure_f.get() >= max_safe_pressure;
    }

    inline bool is_tank1_temp_high() const
    {
        return tank1_temp_f.get() >= max_safe_temp;
    }

    inline bool is_tank2_temp_high() const
    {
        return tank2_temp_f.get() >= max_safe_temp;
    }

    inline bool check_current_state(prop_state_t expected) const
    {
        return expected == static_cast<prop_state_t>(prop_state_f.get());
    }

    static bool is_valid_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4,
                                  unsigned int ctrl_cycles_from_now);

    // Copies the schedule in the writable state fields into Tank2's schedule
    // Precond: schedule should be valid before calling this
    inline void write_tank2_schedule()
    {
        PropulsionSystem.set_schedule(sched_valve1_f.get(), sched_valve2_f.get(), sched_valve3_f.get(),
                                      sched_valve4_f.get());
    }

    // Return True if we are allowed to enter the desired_state
    bool can_enter_state(prop_state_t desired_state) const;

    // https://cornellprod-my.sharepoint.com/personal/saa243_cornell_edu/_layouts/15/Doc.aspx?sourcedoc=%7B10E398A9-3D68-44F8-BE1B-E4FA5DBECBC8%7D&file=Recurring%20Constants%20Review.xlsx&action=default&mobileredirect=true&cid=912c7e7e-8046-4d9c-9ee9-13a59d46f5a6
    TRACKED_CONSTANT_SC(float, max_safe_pressure, 75); // 75 psi
    TRACKED_CONSTANT_SC(float, max_safe_temp, 48);     // 48 C

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
    static PropState_Venting state_venting;
    static PropState_HandlingFault state_handling_fault;
    static PropState_Manual state_manual;
};

// ------------------------------------------------------------------------
// CountdownTimer
// ------------------------------------------------------------------------

// This class is like a countdown timer on a bomb
// It uses units of control cycles
class CountdownTimer
{
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
class PropState
{
public:
    explicit PropState(prop_state_t my_state)
        : this_state(my_state)
    {
    }

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
    static PropController *controller;

    // The only purpose of declaring PropController a friend class is to allow it
    //      to set controller to itself
    friend class PropController;
};

// In this state, Prop ignores all firing requests and hardware faults
// Prop will still read the PropulsionSystem sensors and update the state fields corresponding to those sensors
class PropState_Disabled : public PropState
{
public:
    PropState_Disabled() : PropState(prop_state_t::disabled) {}
    bool can_enter() const override;
    void enter() override;
    prop_state_t evaluate() override;
};

class PropState_Idle : public PropState
{
public:
    PropState_Idle() : PropState(prop_state_t::idle) {}
    bool can_enter() const override;
    void enter() override;
    prop_state_t evaluate() override;
};

// This is the state where we've received a (valid) request to fire.
// NO ONE may change firing parameters once this state is entered. However, this state can be cancelled to go back to Idle
class PropState_AwaitPressurizing : public PropState
{
public:
    PropState_AwaitPressurizing() : PropState(prop_state_t::await_pressurizing) {}
    bool can_enter() const override;
    void enter() override;
    prop_state_t evaluate() override;
};

// This class abstracts the pressurizing behavior, which consists of pressurizing cycles.
// In pressurizing behavior, we have pressurizing cycle, which consists of a filling period
//      followed by a cooling period.
//
// In this class, we have open-close cycles, which consists of an open period
//      followed by a close period.
// In the open period, one of the Tank's valves is opened
// In the close period, all of the Tank's valves are closed
class ActionCycleOpenClose : public PropState
{
public:
    ActionCycleOpenClose(prop_state_t my_state, Devices::Tank &_tank, prop_state_t _success_state, prop_state_t _fail_state) : PropState(my_state),
                                                                                                                               tank(_tank),
                                                                                                                               success_state(_success_state),
                                                                                                                               fail_state(_fail_state) {}
    void enter() override;
    prop_state_t evaluate() override;

protected:
    // Methods and Fields in this section define the parameters of the OpenCloseCycle behavior

    // Defines a success condition for which we should enter the success_state
    virtual bool has_succeeded() const = 0;
    // Defines a fail condition for which we should enter the fail_state
    virtual bool has_failed() const = 0;
    // Returns index of the valve that we should use in the upcoming "open" cycle
    virtual size_t select_valve_index() = 0;
    // Returns the number of control cycles for which the valve should be opened
    virtual size_t get_ctrl_cycles_per_open_period() const = 0;
    // Returns the number of control cycles for which all valves should be closed
    virtual size_t get_ctrl_cycles_per_close_period() const = 0;
    // Return the maximum number of open-close cycles permitted
    // If we exceed this number, then handle_out_of_cycles() will be called
    virtual size_t get_max_cycles() const = 0;
    // Defines what we should do if we have executed for more than get_max_cycles()
    // number of open-close cycles
    virtual prop_state_t handle_out_of_cycles() = 0;
    // Number of open-closed cycles that we have executed since last entering this state
    size_t cycle_count = 0;

private:
    // The tank whose valves we will be opening
    Devices::Tank &tank;
    // The state to enter if has_succeeded() returns True
    prop_state_t success_state;
    // The state to enter if has_failed() returns True
    prop_state_t fail_state;

    // Called when we are in the open period (one of the Tank's selected
    //      valves is currently open)
    prop_state_t handle_valve_is_open();
    // Called when we are in the close period (all valves closed)
    prop_state_t handle_valve_is_close();
    // Starts an open-close cycle
    void start_cycle();

    // The valve index of the valve that we will use in the upcoming or current open cycle
    size_t cur_valve_index;
    // Timer helps keep track of the number of control cycles since entering
    // an open or close period
    CountdownTimer countdown;
    friend class PropController;
};

// A pressurizing cycle consists of a "filling" period and a "cooling period".
//      The firing period is a 1 second duration in which an intertank valve is opened
//      The cooling period is a 10 second duration in which no valve may be opened
// If we have executed 20 consecutive pressurizing cycles and have not yet reached
//      threshold pressure, then this is a fault
// [ cc1 ][ cc2 ][ cc3 ][ cc4 ][ cc5 ][ cc6 ][ cc7 ] <-- control cycles
// [    pressurize cycle (1s) ][    cool off time (10s)                    ...]
class PropState_Pressurizing : public ActionCycleOpenClose
{
public:
    PropState_Pressurizing() : ActionCycleOpenClose(prop_state_t::pressurizing, Tank1, prop_state_t::await_firing, prop_state_t::idle) {}

protected:
    bool can_enter() const override;
    // True if we have reached threshold pressure
    // If the pressurize_failed fault is suppressed, then this is true, when
    // we have executed max_cycles
    bool has_succeeded() const override;
    // True if schedule is no longer valid
    bool has_failed() const override;
    // Whether to use the primary or backup valve
    size_t select_valve_index() override;
    // The number of control cycles per filling period
    size_t get_ctrl_cycles_per_open_period() const override;
    // The number of control cycles per cooling period
    size_t get_ctrl_cycles_per_close_period() const override;
    // Maximum number of pressurizing cycles. If we exceed this, the pressurize_fail fault
    // will be signaled
    size_t get_max_cycles() const override;
    // Called when we have failed to reach threshold_pressure after maximum consecutive pressurizing cycles
    // First, signal pressurize_fail_fault_f. Then evaluate whether this fault has been suppressed by the ground.
    // If it has been suppressed, then continue to pressurize. Otherwise, set Prop to handling_fault
    prop_state_t handle_out_of_cycles() override;
};

class PropState_Firing : public PropState
{
public:
    PropState_Firing() : PropState(prop_state_t::firing) {}
    bool can_enter() const override;
    void enter() override;
    prop_state_t evaluate() override;

private:
    // Returns true if the entire schedule is all zeros
    bool is_schedule_empty() const;
};

class PropState_AwaitFiring : public PropState
{
public:
    PropState_AwaitFiring() : PropState(prop_state_t::await_firing) {}
    bool can_enter() const override;
    void enter() override;
    prop_state_t evaluate() override;

private:
    // Return true if it is time to fire
    bool is_time_to_fire() const;
};

// Two versions of venting: The venting response is basically just the
// pessurizing response
class PropState_Venting : public PropState
{
public:
    PropState_Venting() : PropState(prop_state_t::venting) {}

    bool can_enter() const override;

    void enter() override;

    prop_state_t evaluate() override;

private:
    Devices::Tank &determine_faulted_tank();
};

// HandlingFault consists of autonomous responses to perceived hardware faults
class PropState_HandlingFault : public PropState
{
public:
    PropState_HandlingFault() : PropState(prop_state_t::handling_fault) {}
    bool can_enter() const override;
    void enter() override;
    prop_state_t evaluate() override;

    void handle_pressure_too_high();

    void handle_tank1_temp_too_high();

    void handle_tank2_temp_too_high();
};

class PropState_Manual : public PropState
{
public:
    PropState_Manual() : PropState(prop_state_t::manual) {}
    bool can_enter() const override;
    void enter() override;
    prop_state_t evaluate() override;
};
