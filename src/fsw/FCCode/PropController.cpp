#include <fsw/FCCode/PropController.hpp>

#if (defined(UNIT_TEST) && defined(DESKTOP))
#define DD(f_, ...) std::printf((f_), ##__VA_ARGS__)
#else
#define DD(f_, ...) \
    do              \
    {               \
    } while (0)
#endif

PropController::PropController(StateFieldRegistry &registry, unsigned int offset)
    : TimedControlTask<void>(registry, "prop", offset),
      prop_state_f("prop.state", Serializer<unsigned int>(6)),
      cycles_until_firing("prop.cycles_until_firing", Serializer<unsigned int>(256)),
      sched_valve1_f("prop.sched_valve1", Serializer<unsigned int>(999)),
      sched_valve2_f("prop.sched_valve2", Serializer<unsigned int>(999)),
      sched_valve3_f("prop.sched_valve3", Serializer<unsigned int>(999)),
      sched_valve4_f("prop.sched_valve4", Serializer<unsigned int>(999)),
      sched_intertank1_f("prop.sched_intertank1", Serializer<unsigned int>(999 * 1000)),
      sched_intertank2_f("prop.sched_intertank2", Serializer<unsigned int>(999 * 1000)),

      max_pressurizing_cycles("prop.max_pressurizing_cycles", Serializer<unsigned int>(50)),
      threshold_firing_pressure("prop.threshold_firing_pressure", Serializer<float>(10, 50, 4)),
      ctrl_cycles_per_filling_period("prop.ctrl_cycles_per_filling", Serializer<unsigned int>(50)),
      ctrl_cycles_per_cooling_period("prop.ctrl_cycles_per_cooling", Serializer<unsigned int>(50)),
      tank1_valve("prop.tank1.valve_choice", Serializer<unsigned int>(1)),

      tank2_pressure_f("prop.tank2.pressure", Serializer<float>(0, 150, 4)),
      tank2_temp_f("prop.tank2.temp", Serializer<float>(-200, 200, 4)),
      tank1_temp_f("prop.tank1.temp", Serializer<float>(-200, 200, 4)),

      // We must trust the pressure sensor.
      pressurize_fail_fault_f("prop.pressurize_fail", 0),
      overpressure_fault_f("prop.overpressured", 0),

      tank2_temp_high_fault_f("prop.tank2_temp_high", 0),
      tank1_temp_high_fault_f("prop.tank1_temp_high", 0)
{

    PropulsionSystem.setup();
    add_writable_field(prop_state_f);
    add_writable_field(cycles_until_firing);
    add_writable_field(sched_valve1_f);
    add_writable_field(sched_valve2_f);
    add_writable_field(sched_valve3_f);
    add_writable_field(sched_valve4_f);
    add_writable_field(sched_intertank1_f);
    add_writable_field(sched_intertank2_f);

    add_writable_field(max_pressurizing_cycles);
    add_writable_field(threshold_firing_pressure);
    add_writable_field(ctrl_cycles_per_filling_period);
    add_writable_field(ctrl_cycles_per_cooling_period);
    add_writable_field(tank1_valve);

    add_readable_field(tank2_pressure_f);
    add_readable_field(tank2_temp_f);
    add_readable_field(tank1_temp_f);

    add_fault(pressurize_fail_fault_f);
    add_fault(overpressure_fault_f);
    add_fault(tank2_temp_high_fault_f);
    add_fault(tank1_temp_high_fault_f);

    TRACKED_CONSTANT(unsigned int, max_pressurizing_cycles_ic, 20);
    TRACKED_CONSTANT(float, threshold_firing_pressure_ic, 25.0f);
    TRACKED_CONSTANT(unsigned int, ctrl_cycles_per_filling_period_ic, 1000 / PAN::control_cycle_time_ms);
    TRACKED_CONSTANT(unsigned int, ctrl_cycles_per_cooling_period_ic, 10 * 1000 / PAN::control_cycle_time_ms);
    TRACKED_CONSTANT(unsigned int, tank1_valve_choice_ic, 0);

    max_pressurizing_cycles.set(max_pressurizing_cycles_ic);
    threshold_firing_pressure.set(threshold_firing_pressure_ic);
    ctrl_cycles_per_filling_period.set(ctrl_cycles_per_filling_period_ic);
    ctrl_cycles_per_cooling_period.set(ctrl_cycles_per_cooling_period_ic);
    tank1_valve.set(tank1_valve_choice_ic); // default use 0

    tank2_pressure_f.set(Tank2.get_pressure());
    tank2_temp_f.set(Tank2.get_temp());
    tank1_temp_f.set(Tank1.get_temp());

    PropState::controller = this;
}

PropController *PropState::controller = nullptr;
PropState_Disabled PropController::state_disabled;
PropState_Idle PropController::state_idle;
PropState_AwaitPressurizing PropController::state_await_pressurizing;
PropState_Pressurizing PropController::state_pressurizing;
PropState_AwaitFiring PropController::state_await_firing;
PropState_Firing PropController::state_firing;
// PropState_Venting PropController::state_venting = PropState_Venting();
PropState_HandlingFault PropController::state_handling_fault = PropState_HandlingFault();
PropState_Manual PropController::state_manual;

void PropController::execute()
{
    // Decrement fire_cycle if it is not equal to 0
    if (cycles_until_firing.get() > 0)
        cycles_until_firing.set(cycles_until_firing.get() - 1);

    // We can only enter Handling_Fault if at least one Fault is faulted.
    if (state_handling_fault.can_enter())
    {
        prop_state_f.set(static_cast<unsigned int>(prop_state_t::handling_fault));
    }

    auto current_state = static_cast<prop_state_t>(prop_state_f.get());

    // Evaluate the current state in order to get the next state
    prop_state_t next_state = get_state(current_state).evaluate();

    if (next_state != current_state)
    {
        DD("New state: %u\n", static_cast<unsigned int>(next_state));
        // sanity check
        if (get_state(next_state).can_enter())
        {
            prop_state_f.set(static_cast<unsigned int>(next_state));
            get_state(next_state).enter();
        }
        else
        {
            // This could happen if next_state is IDLE but is_functional() returns false
            DD("Could not enter state!!\n");
            prop_state_f.set(static_cast<unsigned int>(prop_state_t::disabled));
        }
    }

    // Read all the sensors -- we check sensors here instead of earlier in order
    // to spoof sensor readings during HITL tests.
    tank2_pressure_f.set(Tank2.get_pressure());
    tank2_temp_f.set(Tank2.get_temp());
    tank1_temp_f.set(Tank1.get_temp());

    // Check the sensors for faults and signal those faults (if any)
    check_faults();
}

void PropController::check_faults()
{
    if (is_tank2_overpressured())
    {
        overpressure_fault_f.signal();
        DD("Overpressured detected\n");
    }
    if (is_tank2_temp_high())
    {
        tank2_temp_high_fault_f.signal();
        DD("Tank2 Temp High detected: %d\n", Tank2.get_temp());
    }
    if (is_tank1_temp_high())
    {
        tank1_temp_high_fault_f.signal();
        DD("Tank1 Temp High detected: %d\n", Tank1.get_temp());
    }
}

bool PropController::can_enter_state(prop_state_t desired_state) const
{
    return get_state(desired_state).can_enter();
}

PropState &PropController::get_state(prop_state_t state) const
{
    switch (state)
    {
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
    case prop_state_t::handling_fault:
        return state_handling_fault;
    case prop_state_t::manual:
        return state_manual;
    default:
        return state_disabled;
    }
}

bool PropController::is_valid_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4,
                                       unsigned int ctrl_cycles_from_now)
{
    return (v1 < 1000 && v2 < 1000 && v3 < 1000 && v4 < 1000 && ctrl_cycles_from_now > 1);
}

bool PropController::validate_schedule()
{
    return is_valid_schedule(sched_valve1_f.get(), sched_valve2_f.get(), sched_valve3_f.get(), sched_valve4_f.get(),
                             cycles_until_firing.get());
}

bool PropController::is_at_threshold_pressure()
{
    return tank2_pressure_f.get() >= threshold_firing_pressure.get();
}

unsigned int PropController::min_cycles_needed() const
{
    // 20 * fillings + 20 * coolings + 1
    // Instead of 19 coolings, we allow 20 because it may take time for the
    // pressure to stabilize
    return max_pressurizing_cycles.get() *
               (ctrl_cycles_per_filling_period.get() + ctrl_cycles_per_cooling_period.get()) +
           4;
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

void CountdownTimer::tick()
{
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

bool PropState_Disabled::can_enter() const
{
    return true;
}

void PropState_Disabled::enter()
{
    DD("==> entered PropState_Disabled\n");
    // Stop Interval Timer
    // Turn off all valves
    // clear the schedule
    PropulsionSystem.reset();
}

prop_state_t PropState_Disabled::evaluate()
{
    // Call reset() here because we if we are in Disabled because MissionManager set prop_state to disabled,
    // then we will not have managed to call enter()
    PropulsionSystem.reset();
    return this_state;
}

// ------------------------------------------------------------------------
// PropState Idle
// ------------------------------------------------------------------------

bool PropState_Idle::can_enter() const
{
    // TODO: can only enter IDLE if there are no hardware faults
#ifndef DESKTOP
    return PropulsionSystem.is_functional();
#else
    return true;
#endif
}

void PropState_Idle::enter()
{
    DD("==> entered PropState_Idle\n");
}

prop_state_t PropState_Idle::evaluate()
{
    if (controller->can_enter_state(prop_state_t::pressurizing))
        return prop_state_t::pressurizing;

    if (controller->can_enter_state(prop_state_t::await_pressurizing))
        return prop_state_t::await_pressurizing;

    return this_state;
}

// ------------------------------------------------------------------------
// PropState Await_Pressurizing
// ------------------------------------------------------------------------

bool PropState_AwaitPressurizing::can_enter() const
{
    bool was_idle = controller->check_current_state(prop_state_t::idle);
    bool is_schedule_valid = controller->validate_schedule();
    // Enter Await Pressurizing rather than Pressurizing if we have MORE than enough time
    bool more_than_enough_time = controller->cycles_until_firing.get() >= controller->min_cycles_needed();
    return (was_idle && is_schedule_valid && more_than_enough_time);
}

void PropState_AwaitPressurizing::enter()
{
    DD("==> entered PropState_AwaitPressurizing\n");
    // if we can pressurize now, then pressurize now
}

prop_state_t PropState_AwaitPressurizing::evaluate()
{
    if (controller->can_enter_state(prop_state_t::pressurizing))
        return prop_state_t::pressurizing;

    return this_state;
}

// ------------------------------------------------------------------------
// PropState Pressurizing
// ------------------------------------------------------------------------

bool PropState_Pressurizing::can_enter() const
{
    bool was_await_pressurizing = controller->check_current_state(prop_state_t::await_pressurizing);
    bool is_schedule_valid = controller->validate_schedule();
    // Allow from idle because sometimes we can immediately pressurize
    bool was_idle = controller->check_current_state(prop_state_t::idle);
    // It is time to pressurize when we have fewer than twice the min_cycles_needed
    bool is_time_to_pressurize = controller->cycles_until_firing.get() < 2 * controller->min_cycles_needed();
    bool must_immediately_pressurize = controller->cycles_until_firing.get() == controller->min_cycles_needed() - 1;

    return ((was_await_pressurizing && is_time_to_pressurize) || (was_idle && must_immediately_pressurize)) &&
           is_schedule_valid;
}

void PropState_Pressurizing::enter()
{
    DD("==> entered PropState_Pressurizing\n");
    // Set which Tank1 valve to use (default: valve_num = 0)
    valve_num = controller->tank1_valve.get();
    // Reset the pressurizing cycles count to 0
    pressurizing_cycle_count = 0;
    // Reset timer to 0 (just in case)
    countdown.reset_timer();
}

prop_state_t PropState_Pressurizing::evaluate()
{
    // Tick the clock
    countdown.tick();
    // Case 1: Tank2 is at threshold pressure
    if (controller->is_at_threshold_pressure())
    {
        DD("\tTank2 is at threshold pressure!\n");
        PropulsionSystem.close_valve(Tank1, valve_num);
        if (controller->can_enter_state(prop_state_t::await_firing))
        {
            return prop_state_t::await_firing;
        }
        else
        {
            // TODO: sure want to disable?
            DD("\tproblem: entered disabled (%u cycles until firing)!\n", controller->cycles_until_firing.get());
            return prop_state_t::disabled;
        }
    }
    // Case 2: Tank2 is not at threshold pressure
    if (Tank1.is_valve_open(valve_num))
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
    if (countdown.is_timer_zero())
    {
        PropulsionSystem.close_valve(Tank1, valve_num);
        countdown.set_timer_cc(controller->ctrl_cycles_per_cooling_period.get());
    }
    return this_state;
}

prop_state_t PropState_Pressurizing::handle_valve_is_close()
{
    // If we are here, then we are not at threshold pressure.

    if (!controller->validate_schedule())
    {
        DD("\t==> No more cycles left. Going to idle\n");
        return prop_state_t::idle;
    }
    // If we have have pressurized for more than max_pressurizing_cycles
    //      then signal fault
    if (pressurizing_cycle_count >= controller->max_pressurizing_cycles.get())
    {
        // Pressurize_failed event happens at the end of the 20th cooling cycle
        if (countdown.is_timer_zero())
            return handle_pressurize_failed();
    }
    // If we are not on 10s cooldown, then start pressurizing again
    else if (countdown.is_timer_zero())
    {
        start_pressurize_cycle();
        return this_state;
    }
    // Else just chill
    return this_state;
}

prop_state_t PropState_Pressurizing::handle_pressurize_failed()
{
    DD("\tPressurize Failed!\n");
    controller->pressurize_fail_fault_f.signal();
    if (controller->pressurize_fail_fault_f.is_faulted())
    {
        // Go to handling_fault
        return prop_state_t::handling_fault;
    }
    // If the fault is suppressed,
    // then we remain in this state and continue trying to pressurize until we run out of time
    pressurizing_cycle_count = 0;
    return this_state;
}

void PropState_Pressurizing::start_pressurize_cycle()
{
    pressurizing_cycle_count++;
    DD("\tStarting pressurizing cycle: %d.\n\t\t%d cycles until firing!\n", pressurizing_cycle_count,
       controller->cycles_until_firing.get());
    // Open the valve and set the timer to 1 second
    PropulsionSystem.open_valve(Tank1, valve_num);
    // Round normal
    countdown.set_timer_cc(controller->ctrl_cycles_per_filling_period.get());
}

// ------------------------------------------------------------------------
// PropState Await Firing
// ------------------------------------------------------------------------

bool PropState_AwaitFiring::can_enter() const
{
    bool was_pressurizing = controller->check_current_state(prop_state_t::pressurizing);
    return (was_pressurizing && controller->is_at_threshold_pressure() && controller->validate_schedule());
}

void PropState_AwaitFiring::enter()
{
    DD("==> entered PropState_AwaitFiring\n");
}

prop_state_t PropState_AwaitFiring::evaluate()
{
    if (is_time_to_fire())
    {
        // Copy the schedule values from the registry into Tank2
        controller->write_tank2_schedule();
        return prop_state_t::firing;
    }
    return this_state;
}

bool PropState_AwaitFiring::is_time_to_fire() const
{
    return controller->cycles_until_firing.get() == 0;
}

// ------------------------------------------------------------------------
// PropState Firing
// ------------------------------------------------------------------------

bool PropState_Firing::can_enter() const
{
    bool was_await_firing = controller->check_current_state(prop_state_t::await_firing);
    bool is_time_to_fire = controller->cycles_until_firing.get() == 0;
    return was_await_firing && is_time_to_fire;
}

void PropState_Firing::enter()
{
    DD("==> entered PropState_Firing\n");
    PropulsionSystem.start_firing();
}

prop_state_t PropState_Firing::evaluate()
{
    DD("==> PropState_Firing is evaluating\n");
    if (is_schedule_empty())
    {
        PropulsionSystem.disable();
        DD("==> schedule is now empty\n");
        return prop_state_t::idle;
    }
    else
    {
        return this_state;
    }
}

bool PropState_Firing::is_schedule_empty() const
{
    unsigned int remain = 0;
    for (size_t i = 0; i < 4; ++i)
        remain += Tank2.get_schedule_at(i);
    return remain == 0;
}

// ------------------------------------------------------------------------
// PropState HandlingFault
// ------------------------------------------------------------------------

bool PropState_HandlingFault::can_enter() const
{
    // If the current state is disabled, then we must remain in disabled
    if (controller->check_current_state(prop_state_t::disabled))
        return false;
    // Return true if any of the faults are actually faulted. This allows us to ignore bad sensors if the ground
    // decides to suppress/override certain faults
    return (controller->pressurize_fail_fault_f.is_faulted() ||
            controller->overpressure_fault_f.is_faulted() ||
            controller->tank2_temp_high_fault_f.is_faulted() ||
            controller->tank1_temp_high_fault_f.is_faulted());
}

void PropState_HandlingFault::enter()
{
    DD("==> entered PropState_HandlingFault\n");
}

prop_state_t PropState_HandlingFault::evaluate()
{
    // If faults are still signaled, then stay in HandlingFault
    if (can_enter())
    {
        return this_state;
    }
    // Otherwise, return to idle
    return prop_state_t::idle;
}

void PropState_HandlingFault::handle_pressure_too_high()
{
    DD("==> Handling Pressure Too High\n");
    // If this is no longer a problem, then unsignal the fault
    // Check using the actual
}

void PropState_HandlingFault::handle_tank1_temp_too_high()
{
    DD("==> Handling Tank1 Temp Too High\n");
}

void PropState_HandlingFault::handle_tank2_temp_too_high()
{
    DD("==> Handling Tank2 Temp Too High\n");
}

// ------------------------------------------------------------------------
// PropState Manual
// ------------------------------------------------------------------------

bool PropState_Manual::can_enter() const
{
    return true;
}

void PropState_Manual::enter()
{
    PropulsionSystem.reset();
}

void manual_eval(WritableStateField<unsigned int> &sched, Devices::Tank &tank, unsigned int valve_num)
{
    if (sched.get() > 0)
    {
        PropulsionSystem.open_valve(tank, valve_num);
    }
    else
    {
        PropulsionSystem.close_valve(tank, valve_num);
    }
#ifndef DESKTOP
    delayMicroseconds(3000);
#endif
}

prop_state_t PropState_Manual::evaluate()
{
    manual_eval(controller->sched_valve1_f, Tank1, 0);
    manual_eval(controller->sched_valve2_f, Tank1, 1);
    manual_eval(controller->sched_valve3_f, Tank1, 2);
    manual_eval(controller->sched_valve4_f, Tank1, 3);
    manual_eval(controller->sched_intertank1_f, Tank2, 0);
    manual_eval(controller->sched_intertank2_f, Tank2, 1);
    return this_state;
}
