#include <fsw/FCCode/PropController.hpp>

#ifdef FEELS_BAD
#include <iostream>
#define DD(f_, ...) printf((f_), ##__VA_ARGS__)
#else
#define DD(f_, ...)
#endif
#ifdef DESKTOP
size_t g_fake_pressure_cycle_count = 15; // global
#endif

PropController::PropController(StateFieldRegistry &registry, unsigned int offset)
        : TimedControlTask<void>(registry, "prop", offset),
          prop_state_f("prop.state", Serializer<unsigned int>(6)),
          fire_cycle_f("prop.fire_cycle", Serializer<unsigned int>(256)),
          sched_valve1_f("prop.sched_valve1", Serializer<unsigned int>(999)),
          sched_valve2_f("prop.sched_valve2", Serializer<unsigned int>(999)),
          sched_valve3_f("prop.sched_valve3", Serializer<unsigned int>(999)),
          sched_valve4_f("prop.sched_valve4", Serializer<unsigned int>(999)) {

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
PropState_AwaitPressurizing PropController::state_await_pressurizing;
PropState_Pressurizing PropController::state_pressurizing;
PropState_AwaitFiring PropController::state_await_firing;
PropState_Firing PropController::state_firing;
// PropState_Venting PropController::state_venting = PropState_Venting();
// PropState_HandlingFault PropController::state_handling_fault = PropState_HandlingFault();

void PropController::execute() {

    // Decrement fire_cycle if it is not equal to 0
    if (fire_cycle_f.get() != 0)
        fire_cycle_f.set(fire_cycle_f.get() - 1);

    auto current_state = static_cast<prop_state_t>(prop_state_f.get());

    prop_state_t next_state = get_state(current_state).evaluate();
    if (next_state != current_state) {
        // sanity check
        if (get_state(next_state).can_enter()) {
            prop_state_f.set(static_cast<unsigned int>(next_state));
            get_state(next_state).enter();
        } else {
            // This should never happen, if it does, complain a lot.
            // TODO: enter handling fault maybe?
            prop_state_f.set(static_cast<unsigned int>(prop_state_t::disabled));
        }
    }
}

bool PropController::can_enter_state(prop_state_t desired_state) const {
    return get_state(desired_state).can_enter();
}

PropState &PropController::get_state(prop_state_t state) const {
    switch (state) {
        case prop_state_t::disabled:
            return state_disabled;
        case prop_state_t::idle:
            return state_idle;
        case prop_state_t::await_pressurizing:
            return state_await_pressurizing;
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

bool PropController::is_valid_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4,
                                       unsigned int ctrl_cycles_from_now) {
    return (v1 < 1000 && v2 < 1000 && v3 < 1000 && v4 < 1000 && ctrl_cycles_from_now > 1);
}

bool PropController::validate_schedule() {
    return is_valid_schedule(sched_valve1_f.get(), sched_valve2_f.get(), sched_valve3_f.get(), sched_valve4_f.get(),
                             fire_cycle_f.get());
}

bool PropController::is_at_threshold_pressure()
{

#ifdef DESKTOP
    // For testing purposes, say that we are at threshold pressure at pressurizing cycle fake_pressure_cycle_count
    return (state_pressurizing.pressurizing_cycle_count == g_fake_pressure_cycle_count);
#else
    return Tank2.get_pressure() >= threshold_firing_pressure;
#endif
}

// ------------------------------------------------------------------------
// Countdown Timer
// ------------------------------------------------------------------------

bool CountdownTimer::is_timer_zero() const
{
    return cycles_left == 0;
}

void CountdownTimer::set_timer_cc(size_t num_control_cycles)
{
    cycles_left = num_control_cycles;
}

void CountdownTimer::tick() {

    if (cycles_left > 0)
        --cycles_left;
}

void CountdownTimer::reset_timer()
{
    cycles_left = 0;
}

// ------------------------------------------------------------------------
// PropState Disabled
// ------------------------------------------------------------------------

bool PropState_Disabled::can_enter() const {
    return true;
}

void PropState_Disabled::enter()
{
    DD("==> entered PropState_Disabled\n");
}

prop_state_t PropState_Disabled::evaluate() {
    // Call disable here because we might have entered this due to some problem
    if (PropulsionSystem.is_functional())
        PropulsionSystem.disable();
    return this_state;
}

// ------------------------------------------------------------------------
// PropState Idle
// ------------------------------------------------------------------------

bool PropState_Idle::can_enter() const {
    // TODO: can only enter IDLE if there are no hardware faults
    return PropulsionSystem.is_functional();
}

void PropState_Idle::enter() {
    DD("==> entered PropState_Idle\n");
}

prop_state_t PropState_Idle::evaluate() {
    if ( controller->can_enter_state(prop_state_t::pressurizing) )
        return prop_state_t::pressurizing;

    if ( controller->can_enter_state(prop_state_t::await_pressurizing) )
        return prop_state_t::await_pressurizing;

    return this_state;
}

// ------------------------------------------------------------------------
// PropState Await_Pressurizing
// ------------------------------------------------------------------------

bool PropState_AwaitPressurizing::can_enter() const {

    bool was_idle = controller->check_current_state(prop_state_t::idle);
    bool is_schedule_valid = controller->validate_schedule();
    // Enter Await Pressurizing rather than Pressurizing if we have MORE than enough time
    bool more_than_enough_time = controller->fire_cycle_f.get() > PropState_Pressurizing::num_cycles_needed();
    return (was_idle && is_schedule_valid && more_than_enough_time);
}

void PropState_AwaitPressurizing::enter() {
    DD("==> entered PropState_AwaitPressurizing\n");
    // if we can pressurize now, then pressurize now
}

prop_state_t PropState_AwaitPressurizing::evaluate() {

    if ( controller->can_enter_state(prop_state_t::pressurizing) )
        return prop_state_t::pressurizing;

    return this_state;
}

// ------------------------------------------------------------------------
// PropState Pressurizing
// ------------------------------------------------------------------------

bool PropState_Pressurizing::can_enter() const {

    bool was_await_pressurizing = controller->check_current_state(prop_state_t::await_pressurizing);
    // Allow from idle because sometimes we can immediately pressurize
    bool was_idle = controller->check_current_state(prop_state_t::idle);

    return (was_await_pressurizing || was_idle) && is_time_to_pressurize();
}

bool PropState_Pressurizing::is_time_to_pressurize() {
    // It is only time to pressurize when we are 20 pressurizing cycles away
    return controller->fire_cycle_f.get() == num_cycles_needed() ;
}

unsigned int PropState_Pressurizing::num_cycles_needed() {
    // 20 * firing + 19 * coolings
    return PropController::max_pressurizing_cycles * PropState_Pressurizing::ctrl_cycles_per_firing_period +
           (PropController::max_pressurizing_cycles - 1) * PropState_Pressurizing::ctrl_cycles_per_cooling_period;
}

void PropState_Pressurizing::enter() {
    DD("==> entered PropState_Pressurizing\n");
    // Set which Tank1 valve to use (default: valve_num = 0)
    if ( should_use_backup() )
        valve_num = 1;
    // Reset the pressurizing cycles count to 0
    pressurizing_cycle_count = 0;
    // Reset timer to 0 (just in case)
    countdown.reset_timer();
}

bool PropState_Pressurizing::should_use_backup()
{
    // TODO: determine which vault to use
    return false;
}

prop_state_t PropState_Pressurizing::evaluate() {
    // Tick the clock
    countdown.tick();
    // Case 1: Tank2 is at threshold pressure
    if (controller->is_at_threshold_pressure()) {
        DD("\tTank2 is at threshold pressure!\n");
        PropulsionSystem.close_valve(Tank1, valve_num);
        if (controller->can_enter_state(prop_state_t::await_firing)) {
            return prop_state_t::await_firing;
        } else {
            // TODO: sure want to disable?
            DD("\tproblem: entered disabled!\n");
            return prop_state_t::disabled;
        }
    }

    // Case 2: Tank2 is not at threshold pressure
    if ( Tank1.is_valve_open(valve_num) )
    {
        return handle_valve_is_open();
    }
    else
    {
        return handle_valve_is_close();
    }

}

prop_state_t PropState_Pressurizing::handle_valve_is_open()
{
    // If 1 second has past since we opened the valve, then close the valve
    if ( countdown.is_timer_zero() )
    {
        PropulsionSystem.close_valve(Tank1, valve_num);
        countdown.set_timer_cc(ctrl_cycles_per_cooling_period);
    }
    return this_state;
}

prop_state_t PropState_Pressurizing::handle_valve_is_close()
{
    // If we have have pressurized for more than max_pressurizing_cycles
    //      then signal fault
    if (pressurizing_cycle_count >= PropController::max_pressurizing_cycles) {
        return handle_pressurize_failed();
    }
        // If we are not on 10s cooldown, then start pressurizing again
    else if (countdown.is_timer_zero()) {
        start_pressurize_cycle();
        return this_state;
    }
    // Else just chill
    return this_state;
}

prop_state_t PropState_Pressurizing::handle_pressurize_failed()
{
    DD("\tPressurize Failed!\n");
    // TODO: need to set some sort of fault
    return prop_state_t::disabled;
}

void PropState_Pressurizing::start_pressurize_cycle() {
    pressurizing_cycle_count++;
    DD("\tStarting pressurizing cycle: %d.\n\t\t%d fire cycles left!\n", pressurizing_cycle_count,
       controller->fire_cycle_f.get());
    // Open the valve and set the timer to 1 second
    PropulsionSystem.open_valve(Tank1, valve_num);
    // Round normal
    countdown.set_timer_cc(ctrl_cycles_per_firing_period);
}

// ------------------------------------------------------------------------
// PropState Firing
// ------------------------------------------------------------------------

bool PropState_Firing::can_enter() const {
    bool was_await_firing = controller->check_current_state(prop_state_t::await_firing);
    bool is_time_to_fire = controller->fire_cycle_f.get() == 0;
    return was_await_firing && is_time_to_fire;
}

void PropState_Firing::enter() {
    DD("==> entered PropState_Firing\n");
    PropulsionSystem.start_firing();
}

prop_state_t PropState_Firing::evaluate() {
    if (is_schedule_empty()) {
        PropulsionSystem.disable();
        return prop_state_t::idle;
    } else
        return this_state;
}

bool PropState_Firing::is_schedule_empty() const {
    unsigned int remain = 0;
    for (size_t i = 0; i < 4; ++i)
        remain += Tank2.get_schedule_at(i);
    return remain == 0;
}

// ------------------------------------------------------------------------
// PropState Await Firing
// ------------------------------------------------------------------------

bool PropState_AwaitFiring::can_enter() const {
    bool was_pressurizing = controller->check_current_state(prop_state_t::pressurizing);

    return ( was_pressurizing && PropController::is_at_threshold_pressure() && controller->validate_schedule() );
}

void PropState_AwaitFiring::enter() {
    DD("==> entered PropState_AwaitFiring\n");
    // TODO: maybe should be fire_cycle time?
}

prop_state_t PropState_AwaitFiring::evaluate() {
    if (is_time_to_fire()) {
        // Copy the schedule values from the registry into Tank2
        controller->write_tank2_schedule();
        return prop_state_t::firing;
    }
    return this_state;
}

bool PropState_AwaitFiring::is_time_to_fire() const {
    return controller->fire_cycle_f.get() == 0;
}
