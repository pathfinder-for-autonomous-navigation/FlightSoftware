#pragma once

#include <fsw/FCCode/TimedControlTask.hpp>
#include <fsw/FCCode/Drivers/PropulsionSystem.hpp>
#include <fsw/FCCode/prop_state_t.enum>
#include <common/Fault.hpp>
/**
 * Implementation Info:
 * - millisecond to control cycle count conversions take the floor operator - 
 *      change this by changing the constexprs
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
    PropController(StateFieldRegistry &registry);

    TRACKED_CONSTANT(unsigned int, orbit_ccno, PAN::one_day_ccno*(96)/(24*60));

    TRACKED_CONSTANT(unsigned int, max_venting_cycles_ic, 20);
    TRACKED_CONSTANT(unsigned int, max_pressurizing_cycles_ic, 20);
    TRACKED_CONSTANT(float, threshold_firing_pressure_ic, 25.0f);
    TRACKED_CONSTANT(unsigned int, ctrl_cycles_per_filling_period_ic, 1000 / PAN::control_cycle_time_ms);
    TRACKED_CONSTANT(unsigned int, ctrl_cycles_per_cooling_period_ic, 10 * 1000 / PAN::control_cycle_time_ms);

    TRACKED_CONSTANT(unsigned int, tank1_valve_choice_ic, 0);
    TRACKED_CONSTANT(unsigned int, ctrl_cycles_per_close_period_ic, 1000 / PAN::control_cycle_time_ms);


    // ------------------------------------------------------------------------
    // Input Fields
    // ------------------------------------------------------------------------

    WritableStateField<unsigned int> prop_state_f;

    WritableStateField<unsigned int> cycles_until_firing;
    WritableStateField<unsigned int>* sched_valve1_fp;
    WritableStateField<unsigned int>* sched_valve2_fp;
    WritableStateField<unsigned int>* sched_valve3_fp;
    WritableStateField<unsigned int>* sched_valve4_fp;

    WritableStateField<unsigned int> sched_intertank1_f;
    WritableStateField<unsigned int> sched_intertank2_f;

    // ------------------------------------------------------------------------
    // Ground-Modifiable Parameters
    // ------------------------------------------------------------------------

    WritableStateField<unsigned int> max_venting_cycles;
    // Same as ctrl_cycles_per_cooling_period but used for venting
    WritableStateField<unsigned int> ctrl_cycles_per_close_period;

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

    // The number of prop firings that have been executed
    ReadableStateField<unsigned int> num_prop_firings_f;

    Fault pressurize_fail_fault_f; // underpressurized
    Fault overpressure_fault_f;    // overpressurized
    Fault tank2_temp_high_fault_f;
    Fault tank1_temp_high_fault_f;


    // ------------------------------------------------------------------------
    // Public Interface
    // ------------------------------------------------------------------------

    // Evaluates the current state to get the next state
    // If the next state differs from the current state, update our current
    //      state and then call the new state's entry() method
    void execute() override;

    // Checks the sensor values of Tank1 and Tank2.
    // Signals the fault and returns true if any hardware fault is detected
    // Since faults are checked at the end of the control cycle, it takes
    // two control cycles for PropFaultHandler to respond to faults.
    void check_faults();

    // Convenience function that calls is_valid_schedule
    bool validate_schedule();

    // A schedule is valid if no valve schedule exceeds 1000 ms and if
    // ctrl_cycles_from_now > 1
    static bool is_valid_schedule(unsigned int v1,
                                  unsigned int v2,
                                  unsigned int v3,
                                  unsigned int v4,
                                  unsigned int ctrl_cycles_from_now);

    // Minimum of cycles needed to prepare for firing time
    // If cycles_until_firing is set to to this value when we are in Idle,
    // then we will transition directly to Pressurizing
    unsigned int min_cycles_needed() const;

    inline bool is_at_threshold_pressure() const
    {
        return tank2_pressure_f.get() >= threshold_firing_pressure.get();
    }

    inline bool is_tank2_overpressured() const
    {
        return tank2_pressure_f.get() > max_safe_pressure;
    }

    inline bool is_tank1_temp_high() const
    {
        return tank1_temp_f.get() > max_safe_temp;
    }

    inline bool is_tank2_temp_high() const
    {
        return tank2_temp_f.get() > max_safe_temp;
    }

    inline bool check_current_state(prop_state_t expected) const
    {
        return expected == static_cast<prop_state_t>(prop_state_f.get());
    }

    // Used by PropState_AwaitFiring
    // Copies the schedule in the writable state fields into Tank2's schedule
    // Precond: schedule should be valid before calling this
    inline void write_tank2_schedule()
    {
        PropulsionSystem.set_schedule(sched_valve1_f.get(),
                                      sched_valve2_f.get(),
                                      sched_valve3_f.get(),
                                      sched_valve4_f.get());
    }

    // Return True if we are allowed to enter the desired_state
    bool can_enter_state(prop_state_t desired_state) const;

    // https://cornellprod-my.sharepoint.com/personal/saa243_cornell_edu/_layouts/15/Doc.aspx?sourcedoc=%7B10E398A9-3D68-44F8-BE1B-E4FA5DBECBC8%7D&file=Recurring%20Constants%20Review.xlsx&action=default&mobileredirect=true&cid=912c7e7e-8046-4d9c-9ee9-13a59d46f5a6
    TRACKED_CONSTANT_SC(float, max_safe_pressure, 75); // 75 psi
    TRACKED_CONSTANT_SC(float, max_safe_temp, 49);     // 49 C

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

    friend class TestFixture;
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
    explicit PropState(prop_state_t my_state) : this_state(my_state) {}

    // Returns true if the current state of the PropController meets the
    // preconditions for entering this state
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
    friend class PropFaultHandler;
};

// ------------------------------------------------------------------------
// PropState_Disabled
// ------------------------------------------------------------------------

// In this state, Prop ignores all firing requests and hardware faults
// Prop will still read the PropulsionSystem sensors and update the state fields
// corresponding to those sensors
class PropState_Disabled : public PropState
{
public:
    PropState_Disabled() : PropState(prop_state_t::disabled) {}
    bool can_enter() const override;
    void enter() override;
    prop_state_t evaluate() override;
};

// ------------------------------------------------------------------------
// PropState_Idle
// ------------------------------------------------------------------------

// In this state, PropulsionSystem is functional and ready to accept firing
class PropState_Idle : public PropState
{
public:
    PropState_Idle() : PropState(prop_state_t::idle) {}
    // True if PropulsionSystem is functional
    bool can_enter() const override;
    void enter() override;
    // May transition to handling_fault, pressurizing, or await_pressurizing
    prop_state_t evaluate() override;
};

// ------------------------------------------------------------------------
// PropState_AwaitPressurizing
// ------------------------------------------------------------------------

// This is the state where we've received a (valid) request to fire.
class PropState_AwaitPressurizing : public PropState
{
public:
    PropState_AwaitPressurizing() : PropState(prop_state_t::await_pressurizing) {}
    // True if the previous state was Idle and the schedule is valid and we have
    // more than enough time to pressurize
    bool can_enter() const override;
    void enter() override;
    prop_state_t evaluate() override;
};

// ------------------------------------------------------------------------
// AwaitActionCycleOpenCloseressurizing
// ------------------------------------------------------------------------

// This class abstracts the pressurizing behavior, which consists of pressurizing cycles.
// In pressurizing behavior, we have pressurizing cycle, which consists of a filling period
//      followed by a cooling period.
//
// In this class, we have open-close cycles, which consists of an open period
//      followed by a close period.
// In the open (filling) period, one of the Tank's valves is opened
// In the close (cooling) period, all of the Tank's valves are closed
class ActionCycleOpenClose : public PropState
{
public:
    ActionCycleOpenClose(prop_state_t my_state,
                         Devices::Tank *_tank,
                         prop_state_t _success_state,
                         prop_state_t _fail_state) : PropState(my_state),
                                                     p_tank(_tank),
                                                     success_state(_success_state),
                                                     fail_state(_fail_state) {}
    void enter() override;
    prop_state_t evaluate() override;

protected:
    // Methods and Fields in this section define the parameters of the OpenCloseCycle behavior

    // How long we wait in between opening valves
    virtual size_t get_ctrl_cycles_per_close_period() const = 0;
    virtual size_t get_max_cycles() const = 0;
    // Defines a success condition for which we should enter the success_state
    virtual bool has_succeeded() const = 0;
    // Defines a fail condition for which we should enter the fail_state
    virtual bool has_failed() const = 0; // Default return false
    // Returns index of the valve that we should use in the upcoming "open" cycle
    virtual size_t select_valve_index() = 0;
    // Defines what we should do if we have executed for more than max_cycles
    // number of open-close cycles
    virtual prop_state_t handle_out_of_cycles() = 0;
    // Number of open-closed cycles that we have executed since last entering this state
    size_t cycle_count = 0;
    // The tank whose valves we will be opening
    Devices::Tank *p_tank;
    // The valve index of the valve that we will use in the upcoming or current open cycle
    size_t cur_valve_index;

private:
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
    // Timer helps keep track of the number of control cycles since entering
    // an open or close period
    CountdownTimer countdown;
    friend class PropController;
};

// ------------------------------------------------------------------------
// PropState_Pressurizing
// ------------------------------------------------------------------------

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
    PropState_Pressurizing() : ActionCycleOpenClose(prop_state_t::pressurizing,
                                                    &Tank1,
                                                    prop_state_t::await_firing,
                                                    prop_state_t::disabled) {}

protected:
    void enter() override;
    bool can_enter() const override;
    prop_state_t evaluate() override;

    inline size_t get_ctrl_cycles_per_close_period() const override
    {
        return controller->ctrl_cycles_per_cooling_period.get();
    }

    inline size_t get_max_cycles() const override
    {
        return controller->max_pressurizing_cycles.get();
    }

    inline bool has_succeeded() const override
    {
        return controller->is_at_threshold_pressure();
    }

    // True if schedule is no longer valid. This will never occur, but is a good sanity check
    inline bool has_failed() const override
    {
        return !controller->validate_schedule();
    }

    // Whether to use the primary or backup valve
    inline size_t select_valve_index() override
    {
        return controller->tank1_valve.get();
    }

    // Called when we have failed to reach threshold_pressure after maximum
    //      consecutive pressurizing cycles
    // First, signal pressurize_fail_fault_f.
    // If the fault has been suppressed, then go to await_firing
    //      Otherwise, set Prop to handling_fault
    prop_state_t handle_out_of_cycles() override;
};

// ------------------------------------------------------------------------
// PropState_AwaitFiring
// ------------------------------------------------------------------------

class PropState_AwaitFiring : public PropState
{
public:
    PropState_AwaitFiring() : PropState(prop_state_t::await_firing) {}
    // Can enter if the schedule is valid
    bool can_enter() const override;
    void enter() override;
    prop_state_t evaluate() override;

private:
    // Return true if it is time to fire
    bool is_time_to_fire() const;
};

// ------------------------------------------------------------------------
// PropState_Firing
// ------------------------------------------------------------------------

class PropState_Firing : public PropState
{
public:
    PropState_Firing() : PropState(prop_state_t::firing) {}
    // True if the previous state was await_firing and it is time to fire
    bool can_enter() const override;
    void enter() override;
    prop_state_t evaluate() override;

private:
    // Returns true if the entire schedule is all zeros
    bool is_schedule_empty() const;
};

// ------------------------------------------------------------------------
// PropState_HandlingFault
// ------------------------------------------------------------------------

// HandlingFault consists of autonomous responses to perceived hardware faults
class PropState_HandlingFault : public PropState
{
public:
    PropState_HandlingFault() : PropState(prop_state_t::handling_fault) {}
    // True if the current state is not disabled
    //      and any of the four faults are faulted
    bool can_enter() const override;
    void enter() override;
    prop_state_t evaluate() override;
};

// ------------------------------------------------------------------------
// PropState_Venting
// ------------------------------------------------------------------------

// Two versions of venting: The venting response is basically just the
// pressurizing response
class PropState_Venting : public ActionCycleOpenClose
{
public:
    // Default to tank2 but enter() will determine the tank at runtime
    PropState_Venting() : ActionCycleOpenClose(prop_state_t::venting,
                                               &Tank1,
                                               prop_state_t::idle,
                                               prop_state_t::disabled) {}

protected:
    void enter() override;
    // True if any of the faults are faulted EXCEPT for pressurize_failed_f
    bool can_enter() const override;
    inline size_t get_ctrl_cycles_per_close_period() const override
    {
        return controller->ctrl_cycles_per_close_period.get();
    }

    inline size_t get_max_cycles() const override
    {
        return controller->max_venting_cycles.get();
    }
    // True if we are no longer faulted
    bool has_succeeded() const override;
    // PropFaultHandler determines whether Venting has failed, so just return false
    inline bool has_failed() const override
    {
        return false;
    }
    // Determine which valves to open
    size_t select_valve_index() override;
    // Called if we have ran out of cycles and we are still faulted
    prop_state_t handle_out_of_cycles() override;

    inline bool tank1_wants_to_vent() const
    {
        return controller->tank1_temp_high_fault_f.is_faulted();
    }

    inline bool tank2_wants_to_vent() const
    {
        return (controller->tank2_temp_high_fault_f.is_faulted() ||
                controller->overpressure_fault_f.is_faulted());
    }

private:
    // If both Tanks want to vent, then we take turns venting for 1 open-close
    // cycle
    unsigned int determine_faulted_tank();
    // The purpose of this is to let tank2 rotate valves so that it's not just
    // opening the same valve
    size_t saved_tank2_valve_choice = 0;

    // tank_choice := 1 for Tank1, 2 for Tank2
    unsigned int tank_choice = 1;
    friend class TestFixture;
};

// ------------------------------------------------------------------------
// PropState_Manual
// ------------------------------------------------------------------------
class PropState_Manual : public PropState
{
public:
    PropState_Manual() : PropState(prop_state_t::manual) {}

    bool can_enter() const override;

    void enter() override;

    prop_state_t evaluate() override;
};
