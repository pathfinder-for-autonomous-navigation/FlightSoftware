#pragma once
#include <fsw/FCCode/TimedControlTask.hpp>
#include <fsw/FCCode/Drivers/PropulsionSystem.hpp>
#include <fsw/FCCode/prop_state_t.enum>

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
    // State Handling Functions
    // ------------------------------------------------------------------------

    // Prop is only willing to transition to idle
    void dispatch_disabled();
    // Prop is willing to start pressurizing
    void dispatch_idle();
    // Tank1 valves are open
    void dispatch_pressurizing();
    // Both Tank1 and Tank2 valves are open
    void dispatch_venting();
    // Tank2 valves are currently open
    void dispatch_firing();
    // Tank2 pressure must be at the threshold_pressure but has not yet fired
    void dispatch_await_firing();
    
    void dispatch_handling_fault();
    

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
        return Tank2.get_pressure() >= threshold_firing_pressure;
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
// Propulsion States
// ------------------------------------------------------------------------

// abstract PropState class
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
};

// A pressurizing cycle is a 1 second duration in which an intertank valve is opened
// A single pressurizing cycle will span multiple control cycles
// If we have executed 20 consecutive pressurizing cycles and have not yet reached
// threshold pressure, then this is a fault
class PropState_Pressurizing : public PropState
{
public:
    PropState_Pressurizing() : PropState(prop_state_t::pressurizing) {}
    bool can_enter() override;
    prop_state_t evaluate() override;

private:
    // Called when prop is currently in a pressurizing cycle (i.e. valve is open)
    void handle_currently_pressurizing();
    // Called when prop has failed to reach threshod_pressure after maximum consecutive pressurizing cycles
    void handle_pressurize_failed();
    // Starts another pressurization cycle
    void start_pressurize_cycle();
    // Returns true if we should use the backup valve for pressurizing
    static bool should_use_backup();

    // The number of pressurizing cycles we have executed since entering this state
    unsigned int current_cycle = 0;
    // 1 if we are using the backup valve and 0 otherwise
    bool valve_num = 0;
    unsigned int cycle_start_time = 0;
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