#include <fsw/FCCode/PropController.hpp>
#ifdef DESKTOP
#include <iostream>
using namespace std;
#endif
PropController::PropController(StateFieldRegistry& registry, unsigned int offset)
: TimedControlTask<void>(registry, "prop", offset),
    prop_state_f("prop.state", Serializer<unsigned int>(6)),
    fire_cycle_f("prop.fire_cycle", Serializer<unsigned int>(256)),
    sched_valve1_f("prop.sched_valve1", Serializer<unsigned int>(999)),
    sched_valve2_f("prop.sched_valve2", Serializer<unsigned int>(999)),
    sched_valve3_f("prop.sched_valve3", Serializer<unsigned int>(999)),
    sched_valve4_f("prop.sched_valve4", Serializer<unsigned int>(999))
{

    add_writable_field(prop_state_f);
    add_writable_field(fire_cycle_f);
    add_writable_field(sched_valve1_f);
    add_writable_field(sched_valve2_f);
    add_writable_field(sched_valve3_f);
    add_writable_field(sched_valve4_f);

    PropState::controller = this;
}

PropController* PropState::controller = nullptr;
PropState_Disabled PropController::state_disabled = PropState_Disabled();
PropState_Idle PropController::state_idle = PropState_Idle();
PropState_Pressurizing PropController::state_pressurizing = PropState_Pressurizing();
PropState_AwaitFiring PropController::state_await_firing = PropState_AwaitFiring();
PropState_Firing PropController::state_firing = PropState_Firing();
// PropState_Venting PropController::state_venting = PropState_Venting();
// PropState_HandlingFault PropController::state_handling_fault = PropState_HandlingFault();

void PropController::execute()
{
    prop_state_t current_state = static_cast<prop_state_t>(prop_state_f.get());
    
    prop_state_t next_state = get_state(current_state).evaluate();
    if (next_state != current_state)
    {
        if ( get_state(next_state).can_enter() )
        {
            prop_state_f.set(static_cast<unsigned int>(next_state));
        }
        else
        {
            // TODO: 
        }
    }
    // Decrement fire_cycle if it is not equal to 0
    if (fire_cycle_f.get() != 0)
        fire_cycle_f.set(fire_cycle_f.get() - 1);
}

PropState& PropController::get_state(prop_state_t state)
{
  switch( state )
  {
    case prop_state_t::disabled:
        return state_disabled;
    case prop_state_t::idle:
        return state_idle;
    case prop_state_t::pressurizing:
        return state_pressurizing;
    case prop_state_t::await_firing:
        return state_await_firing;
    case prop_state_t::firing:
        return state_firing;
    // case prop_state_t::venting:
    //     return state_venting;
    // case prop_state_t::handling_fault:
    //     return state_handling_fault;
    default:
        return state_disabled;
  }
}

bool PropController::is_valid_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4, unsigned int ctrl_cycles_from_now)
{
  if (ctrl_cycles_from_now <= 20)
    return false;
  if (v1 > 1000 || v2 > 1000 || v3 > 1000 || v4 > 1000)
    return false;
  return true;
}

bool PropController::validate_schedule()
{
    return is_valid_schedule(sched_valve1_f.get(), sched_valve2_f.get(), sched_valve3_f.get(), sched_valve4_f.get(), fire_cycle_f.get());
}

void PropController::set_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4, unsigned int ctrl_cycles_from_now)
{
  if ( !is_valid_schedule(v1, v2, v3, v4, ctrl_cycles_from_now) )
    return;
  sched_valve1_f.set(v1);
  sched_valve2_f.set(v2);
  sched_valve3_f.set(v3);
  sched_valve4_f.set(v4);
  fire_cycle_f.set(ctrl_cycles_from_now);
    // cout << "hit 106" << endl;
}

// ------------------------------------------------------------------------
// PropState Disabled
// ------------------------------------------------------------------------

bool PropState_Disabled::can_enter()
{
    return true;
}

prop_state_t PropState_Disabled::evaluate()
{
    return this_state;
}

// ------------------------------------------------------------------------
// PropState Idle
// ------------------------------------------------------------------------

bool PropState_Idle::can_enter()
{
    if( !controller->check_current_state(prop_state_t::disabled) )
        return false;
     // TODO: can only enter IDLE if there are no hardware faults
    return true;
}

prop_state_t PropState_Idle::evaluate()
{
    // check schedule
    if ( controller->validate_schedule() )
        return prop_state_t::pressurizing;
    // if there is a feasible schedule then check time
    // TODO: 
    // if it is time to pressurize then transition to pressurizing

    // otherwise, stay in idle
    return this_state;
}

// ------------------------------------------------------------------------
// PropState Pressurzing
// ------------------------------------------------------------------------

bool PropState_Pressurizing::can_enter()
{
    // only allow entrance from prop_state::idle
    if ( !controller->check_current_state(prop_state_t::idle) )
        return false;
    // Set which Tank1 valve to use (default: valve_num = 0)
    if (should_use_backup())
        valve_num = 1;
    // Reset the pressurizing cycles count to 0
    current_cycle = 0;
    return true;
}

bool PropState_Pressurizing::should_use_backup()
{
    // TODO: determine which vault to use
    return false;
}

prop_state_t PropState_Pressurizing::evaluate()
{
    if (controller->is_at_threshold_pressure())
    {
        PropulsionSystem.close_valve(Tank1, valve_num);
        return prop_state_t::await_firing;
    }
    // Tank2 is not at threshold pressure
    if (Tank1.is_valve_open(valve_num))
    { 
        handle_currently_pressurizing();
        return this_state;
    }
    // Tank is not at threshold pressure and is not opened
    if (current_cycle > PropController::max_pressurizing_cycles)
    {
        handle_pressurize_failed();
        return prop_state_t::handling_fault;
    }
    else
    {
        start_pressurize_cycle();
        return this_state;
    }
}

void PropState_Pressurizing::handle_currently_pressurizing()
{
    // check if 1s has passed and close valve if it has
    // TODO: fix cycle_duration_ms
    
    // Assuming that this subtraction is always safe -- TODO: confirm this?
    // if ()
    // {
    //     PropulsionSystem.close_valve(Tank1, valve_num);
    // }
}

void PropState_Pressurizing::handle_pressurize_failed()
{
    // TODO need to set some sort of fault
}

void PropState_Pressurizing::start_pressurize_cycle()
{
    // TODO: 
    // Make the start of this cycle
    cycle_start_time = 0;
    // Increment the cycle count
    current_cycle++;
    // Open the valve
    PropulsionSystem.open_valve(Tank1, valve_num);
}

// ------------------------------------------------------------------------
// PropState Firing
// ------------------------------------------------------------------------

bool PropState_Firing::can_enter()
{
    // only allow entrance from prop_state::await_firing
    if ( !controller->check_current_state(prop_state_t::await_firing) )
        return false;
    // Start the IntervalTimer
    PropulsionSystem.start_firing();
    return true;
}

prop_state_t PropState_Firing::evaluate()
{
    // Check the schedule to see if we are done
    if (is_schedule_empty())
        return prop_state_t::idle;
    else
        return this_state;
}

bool PropState_Firing::is_schedule_empty()
{
    unsigned int remain = 0;
    for (size_t i = 0; i < 4; ++i)
        remain += Tank2.get_schedule_at(i);
    return remain == 0;
}

// ------------------------------------------------------------------------
// PropState Await Firing
// ------------------------------------------------------------------------

bool PropState_AwaitFiring::can_enter()
{
    // only allow entrance from pressurizing or idle
    if ( !controller->check_current_state(prop_state_t::pressurizing) 
            || !controller->check_current_state(prop_state_t::idle) )
        return false;

    // tank should be pressurized
    if ( !PropController::is_at_threshold_pressure() )
        return false;

    // there should be a schedule and a firing time
    if ( !controller->validate_schedule() )
        return false;

    return true;
}

prop_state_t PropState_AwaitFiring::evaluate()
{
    // if we are within a control cycle of firing time then fire
    if ( is_time_to_fire() )
        return prop_state_t::firing;
    
    return this_state;
}

bool PropState_AwaitFiring::is_time_to_fire()
{
    return controller->fire_cycle_f.get() == 0;
}
