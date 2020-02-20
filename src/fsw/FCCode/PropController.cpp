#include <fsw/FCCode/PropController.hpp>

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
PropState_Disabled PropController::state_disabled;
PropState_Idle PropController::state_idle;
PropState_Pressurizing PropController::state_pressurizing;
PropState_AwaitFiring PropController::state_await_firing;
PropState_Firing PropController::state_firing;
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
            // TODO: what to do if tried to enter a state and was not allowed?
        }
    }
    // Tick all the countdown timers
    CountdownTimer::tick();

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
    return ! (v1 > 1000 || v2 > 1000 || v3 > 1000 || v4 > 1000 || ctrl_cycles_from_now < 1);
}

bool PropController::validate_schedule()
{
    return is_valid_schedule(sched_valve1_f.get(), sched_valve2_f.get(), sched_valve3_f.get(), sched_valve4_f.get(), fire_cycle_f.get());
}

bool PropController::is_at_threshold_pressure()
{
#ifdef DESKTOP
    return (state_pressurizing.pressurizing_cycle_count == 15);
#else
    return Tank2.get_pressure() >= threshold_firing_pressure;
#endif
}

// ------------------------------------------------------------------------
// Countdown Timer
// ------------------------------------------------------------------------

std::vector<CountdownTimer*> CountdownTimer::tick_list;
CountdownTimer::CountdownTimer()
{
    tick_list.push_back(this);
}

bool CountdownTimer::is_timer_zero() const
{
    return cycles_left == 0;
}

void CountdownTimer::set_timer_cc(size_t num_control_cycles)
{
    cycles_left = num_control_cycles;
}

void CountdownTimer::set_timer_ms(size_t num_ms)
{
    // convert from ms to control cycles
    size_t num_cc = num_ms/PAN::control_cycle_time_ms;
    cycles_left = num_cc;
}

void CountdownTimer::tick()
{
    // TODO: is there a better way to do this? This is kinda dangerous
    for (CountdownTimer* c : tick_list)
    {
        if (c->cycles_left > 0)
            --c->cycles_left;
    }
}

void CountdownTimer::reset_timer()
{
    cycles_left = 0;
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
     // TODO: can only enter IDLE if there are no hardware faults
    return true;
}

prop_state_t PropState_Idle::evaluate()
{
    if ( is_time_to_pressurize() )
    {
        return prop_state_t::pressurizing;
    }

    return this_state;
}

bool PropState_Idle::is_time_to_pressurize() const
{
    bool is_within_pressurizing_time = controller->fire_cycle_f.get() < num_cycles_within_firing_to_pressurize + PropState_Pressurizing::num_cycles_needed();
    bool is_schedule_valid = controller->validate_schedule();
    bool has_enough_time = PropState_Pressurizing::can_pressurize_in_time();
    return (is_within_pressurizing_time && is_schedule_valid && has_enough_time);
}

// ------------------------------------------------------------------------
// PropState Pressurzing
// ------------------------------------------------------------------------

bool PropState_Pressurizing::can_enter()
{
    // Only allow entrance from prop_state::idle
    if ( !controller->check_current_state(prop_state_t::idle) )
        return false;
    // Set which Tank1 valve to use (default: valve_num = 0)
    if ( should_use_backup() )
        valve_num = 1;
    // Reset the pressurizing cycles count to 0
    pressurizing_cycle_count = 0;
    // Reset timer to 0 (just in case)
    countdown.reset_timer();
    return true;
}

bool PropState_Pressurizing::can_pressurize_in_time()
{
    return controller->fire_cycle_f.get() >= num_cycles_needed();
}

unsigned int PropState_Pressurizing::num_cycles_needed()
{
    return ( PropController::max_pressurizing_cycles + 1 )
        * PropState_Pressurizing::ctrl_cycles_per_pressurizing_cycle;  
}

bool PropState_Pressurizing::should_use_backup()
{
    // TODO: determine which vault to use
    return false;
}

prop_state_t PropState_Pressurizing::evaluate()
{
    // Case 1: Tank2 is at threshold pressure
    if (controller->is_at_threshold_pressure())
    {
        PropulsionSystem.close_valve(Tank1, valve_num);
        return prop_state_t::await_firing;
    }

    // Case 2: Tank2 is not at threshold pressure
    if ( Tank1.is_valve_open(valve_num) )
        handle_valve_is_open();
    else
        handle_valve_is_close();

    return this_state;
}

void PropState_Pressurizing::handle_valve_is_open()
{
    // If 1 second has past since we opened the valve, then close the valve
    if ( countdown.is_timer_zero() )
    {
        PropulsionSystem.close_valve(Tank1, valve_num);
        // Start cooldown timer
        countdown.set_timer_ms(cooling_duration_ms);
    }
}

void PropState_Pressurizing::handle_valve_is_close()
{
    // If we have have pressurized for more than max_pressurizing_cycles
    //      then signal fault
    if ( pressurizing_cycle_count > PropController::max_pressurizing_cycles )
    {
        handle_pressurize_failed();
    }
    // If we are not on 10s cooldown, then start pressurizing again
    else if( countdown.is_timer_zero() )
    {
        start_pressurize_cycle();
    }
}

void PropState_Pressurizing::handle_pressurize_failed()
{
    // TODO: need to set some sort of fault
}

void PropState_Pressurizing::start_pressurize_cycle()
{
    pressurizing_cycle_count++;

    // Open the valve and set the timer to 1 second
    PropulsionSystem.open_valve(Tank1, valve_num);
    countdown.set_timer_ms(firing_duration_ms);
}

// ------------------------------------------------------------------------
// PropState Firing
// ------------------------------------------------------------------------

bool PropState_Firing::can_enter()
{
    // only allow entrance from prop_state::await_firing
    if ( !controller->check_current_state(prop_state_t::await_firing) )
        return false;

    PropulsionSystem.start_firing();
    return true;
}

prop_state_t PropState_Firing::evaluate()
{
    if ( is_schedule_empty() )
    {
        PropulsionSystem.disable();
        return prop_state_t::idle;   
    }
    else
        return this_state;
}

bool PropState_Firing::is_schedule_empty() const
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
            && !controller->check_current_state(prop_state_t::idle) )
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
    if ( is_time_to_fire() )
    {
        // Copy the schedule values from the registry into Tank2
        controller->write_tank2_schedule();
        return prop_state_t::firing;
    }
    return this_state;
}

bool PropState_AwaitFiring::is_time_to_fire() const
{
    return controller->fire_cycle_f.get() == 0;
}
