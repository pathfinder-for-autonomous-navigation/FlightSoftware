#include <fsw/FCCode/PropController.hpp>

#if (defined(UNIT_TEST) && defined(DESKTOP))
#define DD(f_, ...) std::printf((f_), ##__VA_ARGS__)
#else
#define DD(f_, ...) \
    do              \
    {               \
    } while (0)
#endif

/**
 * @brief Constructor for Prop Controller to initialize the state fields with unsigned int values
 * 
 * The number of bits for the serializer is assigned to each statefield. 
 * 
 * @param registry The registry that holds all the statefields
 * @param offset The bit values that the serializer will contain
 */
PropController::PropController(StateFieldRegistry &registry, unsigned int offset)
    : TimedControlTask<void>(registry, "prop", offset),
      prop_state_f("prop.state", Serializer<unsigned int>(9)),
      cycles_until_firing("prop.cycles_until_firing", Serializer<unsigned int>(orbit_ccno)),
      sched_valve1_f("prop.sched_valve1", Serializer<unsigned int>(999)),
      sched_valve2_f("prop.sched_valve2", Serializer<unsigned int>(999)),
      sched_valve3_f("prop.sched_valve3", Serializer<unsigned int>(999)),
      sched_valve4_f("prop.sched_valve4", Serializer<unsigned int>(999)),
      sched_intertank1_f("prop.sched_intertank1", Serializer<unsigned int>(2000)),
      sched_intertank2_f("prop.sched_intertank2", Serializer<unsigned int>(2000)),

      max_venting_cycles("prop.max_venting_cycles", Serializer<unsigned int>(50)),
      ctrl_cycles_per_close_period("prop.ctrl_cycles_per_closing", Serializer<unsigned int>(50)),

      max_pressurizing_cycles("prop.max_pressurizing_cycles", Serializer<unsigned int>(50)),
      threshold_firing_pressure("prop.threshold_firing_pressure", Serializer<float>(10, 50, 6)),
      ctrl_cycles_per_filling_period("prop.ctrl_cycles_per_filling", Serializer<unsigned int>(25)),
      ctrl_cycles_per_cooling_period("prop.ctrl_cycles_per_cooling", Serializer<unsigned int>(100)),
      tank1_valve("prop.tank1.valve_choice", Serializer<unsigned int>(1)),
      tank2_pressure_f("prop.tank2.pressure", Serializer<float>(0, 150, 8)),
      // https://docs.google.com/spreadsheets/d/11-WSDgYckQGl1wP8uMO4zKPSDAdWwGQj/edit#gid=422064948
      tank2_temp_f("prop.tank2.temp", Serializer<float>(-60, 155, 8)),
      tank1_temp_f("prop.tank1.temp", Serializer<float>(-60, 155, 8)),
      num_prop_firings_f("prop.num_prop_firings", Serializer<unsigned int>()),

      // We must trust the pressure sensor.
      pressurize_fail_fault_f("prop.pressurize_fail", 0),
      overpressure_fault_f("prop.overpressured", 10),
      tank2_temp_high_fault_f("prop.tank2_temp_high", 10),
      tank1_temp_high_fault_f("prop.tank1_temp_high", 10)
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

    add_writable_field(max_venting_cycles);
    add_writable_field(ctrl_cycles_per_close_period);

    add_writable_field(max_pressurizing_cycles);
    add_writable_field(threshold_firing_pressure);
    add_writable_field(ctrl_cycles_per_filling_period);
    add_writable_field(ctrl_cycles_per_cooling_period);
    add_writable_field(tank1_valve);

    add_readable_field(tank2_pressure_f);
    add_readable_field(tank2_temp_f);
    add_readable_field(tank1_temp_f);
    add_readable_field(num_prop_firings_f);

    add_fault(pressurize_fail_fault_f);
    add_fault(overpressure_fault_f);
    add_fault(tank2_temp_high_fault_f);
    add_fault(tank1_temp_high_fault_f);

    max_pressurizing_cycles.set(max_pressurizing_cycles_ic);
    max_venting_cycles.set(max_venting_cycles_ic);
    ctrl_cycles_per_close_period.set(ctrl_cycles_per_close_period_ic);

    threshold_firing_pressure.set(threshold_firing_pressure_ic);
    ctrl_cycles_per_filling_period.set(ctrl_cycles_per_filling_period_ic);
    ctrl_cycles_per_cooling_period.set(ctrl_cycles_per_cooling_period_ic);
    tank1_valve.set(tank1_valve_choice_ic); // default use 0
    tank2_pressure_f.set(Tank2.get_pressure());
    tank2_temp_f.set(Tank2.get_temp());
    tank1_temp_f.set(Tank1.get_temp());
    num_prop_firings_f.set(0);

    PropState::controller = this;
}

PropController *PropState::controller = nullptr;
PropState_Disabled PropController::state_disabled;
PropState_Idle PropController::state_idle;
PropState_AwaitPressurizing PropController::state_await_pressurizing;
PropState_Pressurizing PropController::state_pressurizing;
PropState_AwaitFiring PropController::state_await_firing;
PropState_Firing PropController::state_firing;
PropState_Venting PropController::state_venting;
PropState_HandlingFault PropController::state_handling_fault;
PropState_Manual PropController::state_manual;

/**
 * Utilizes helper function written later to check for faults.
 * 
 * Amount of cycles gets decremented to 0 and when 0, prop system stops firing.
 * 
 * Change in state is verified and if not done yet, is forced by conditional statement. If still not working, a reboot is attempted to powercycle the satellite.
 * 
 * Sensors are read immediately in order to spoof data for HITL tests.
 * 
 */
void PropController::execute()
{
    check_faults();
    // Decrement fire_cycle if it is not equal to 0
    if (cycles_until_firing.get() > 0)
        cycles_until_firing.set(cycles_until_firing.get() - 1);

    auto current_state = static_cast<prop_state_t>(prop_state_f.get());

    // Evaluate the current state in order to get the next state
    prop_state_t next_state = get_state(current_state).evaluate();

    if (next_state != current_state)
    {
        DD("[*] Attempt transition to New state: %u\n", static_cast<unsigned int>(next_state));
        // sanity check
        if (get_state(next_state).can_enter())
        {
            DD("[*] Transitioning to New state: %u\n", static_cast<unsigned int>(next_state));
            prop_state_f.set(static_cast<unsigned int>(next_state));
            get_state(next_state).enter();
        }
        else
        {
            // This could happen if next_state is idle but is_functional returns false
            // or if powercycling is happening 
            DD("[-] Could not enter state!!\n\n\n");
            prop_state_f.set(static_cast<unsigned int>(prop_state_t::disabled));
        }
    }
    // Read all the sensors -- check sensors here instead of earlier in order
    // to spoof sensor readings during HITL tests.
    tank2_pressure_f.set(Tank2.get_pressure());
    tank2_temp_f.set(Tank2.get_temp());
    tank1_temp_f.set(Tank1.get_temp());
}
/**
 * Calls on helper functions written that check if the parameterized values for the tanks are too high or overpressured by evaluating
 * their return statement calls signal if the flag is true and unsignal if the flag is false.
 * 
 * Flags are in PropController.hpp.
 */
void PropController::check_faults()
{
    overpressure_fault_f.evaluate(is_tank2_overpressured());
    tank2_temp_high_fault_f.evaluate(is_tank2_temp_high());
    tank1_temp_high_fault_f.evaluate(is_tank1_temp_high());
}

/**
 * Checks whether a particular state can be entered by checking whther the propstate is disabled. 
 * Checked with a helper function in PropController.cpp
 * 
 * @param desired_state Used with argument "next_state" in execute() 
 * @return Type: Boolean. For whether the state can be entered.
 */
bool PropController::can_enter_state(prop_state_t desired_state) const
{
    return get_state(desired_state).can_enter();
}

/**
 * Function that contains a switch conditional and determines the state based off the prop.state value,
 * determined off an enum selection in prop_state_t.enum
 * 
 * @param state input state for which prop.state's value is determined off of
 * @return PropState& Gives the memory address of the state
 */
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
    case prop_state_t::venting:
        return state_venting;
    case prop_state_t::handling_fault:
        return state_handling_fault;
    case prop_state_t::manual:
        return state_manual;
    default:
        return state_disabled;
    }
}

/**
 * Function that uses helper function is_valid_schedule to check the schedules of each valve and to
 * make sure that the cycles until firing is above 1.
 * 
 * @return true if a valid schedule
 * @return false if not a valid schedule
 */
bool PropController::validate_schedule()
{
    return is_valid_schedule(sched_valve1_f.get(),
                             sched_valve2_f.get(),
                             sched_valve3_f.get(),
                             sched_valve4_f.get(),
                             cycles_until_firing.get());
}

/**
 * Helper function for valid_schedule that checks if each schedule value is valid itself using a boolean 
 * statement.
 * 
 * @param v1 Schedule for the first valve
 * @param v2 Schedule for the second valve
 * @param v3 Schedule for the third valve
 * @param v4 Schedule for the fourth valve
 * @param ctrl_cycles_from_now The number of cycles from now
 * @return true if is a valid schedule
 * @return false if is not a valid schedule
 */
bool PropController::is_valid_schedule(unsigned int v1,
                                       unsigned int v2,
                                       unsigned int v3,
                                       unsigned int v4,
                                       unsigned int ctrl_cycles_from_now)
{
    return (v1 <= 1000 && v2 <= 1000 && v3 <= 1000 && v4 <= 1000 && ctrl_cycles_from_now > 1);
}

/**
 * Helper function that is used in pressurization functions to calculate the minimum number of cycles that are needed.
 * This is done by adding the maximum number of pressuring cycles + the control cycles per filling period + 
 * control cycles per cooling period.
 * 
 * @return unsigned int: The minimum number of cycles that are needed
 */
unsigned int PropController::min_cycles_needed() const
{
    return max_pressurizing_cycles.get() *
               (ctrl_cycles_per_filling_period.get() +
                ctrl_cycles_per_cooling_period.get()) +
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
    DD("[*] ==> entered PropState_Disabled\n");
    // Stop Interval Timer, Turn off all valves, clear the schedule
    PropulsionSystem.reset();
}

prop_state_t PropState_Disabled::evaluate()
{
    // Call reset() here because we if we are in Disabled because MissionManager
    // set prop_state to disabled, then we will not have managed to call enter()
    PropulsionSystem.reset();
    return this_state;
}

// ------------------------------------------------------------------------
// PropState Idle
// ------------------------------------------------------------------------

bool PropState_Idle::can_enter() const
{
#ifndef DESKTOP
    return PropulsionSystem.is_functional();
#else
    return true;
#endif
}

void PropState_Idle::enter()
{
    DD("[*] ==> entered PropState_Idle\n");
}

prop_state_t PropState_Idle::evaluate()
{
    if (controller->can_enter_state(prop_state_t::handling_fault))
        return prop_state_t::handling_fault;
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
    bool more_than_enough_time =
        controller->cycles_until_firing.get() >= controller->min_cycles_needed();
    bool is_functional = PropulsionSystem.is_functional();

    return (was_idle && is_schedule_valid && more_than_enough_time && is_functional);
}

void PropState_AwaitPressurizing::enter()
{
    DD("[*] ==> entered PropState_AwaitPressurizing\n");
}

prop_state_t PropState_AwaitPressurizing::evaluate()
{
    if (controller->can_enter_state(prop_state_t::handling_fault))
        return prop_state_t::handling_fault;
    if (controller->can_enter_state(prop_state_t::pressurizing))
        return prop_state_t::pressurizing;
    return this_state;
}

// ------------------------------------------------------------------------
// PropState ActionCycleOpenClose
// ------------------------------------------------------------------------

void ActionCycleOpenClose::enter()
{
    // Reset cycle count to 0 and reset timer (just in case)
    cycle_count = 0;
    countdown.reset_timer();
}

prop_state_t ActionCycleOpenClose::evaluate()
{
    // Tick the clock
    countdown.tick();
    if (has_succeeded())
    {
        DD("[+] ActionCycleOpenClose has achieved goal\n");
        PropulsionSystem.close_valve(*p_tank, cur_valve_index);
        return success_state;
    }
    if (has_failed())
    {
        DD("[!] ActionCycleOpenClose has failed\n");
        return fail_state;
    }

    if (p_tank->is_valve_open(cur_valve_index))
        return handle_valve_is_open();
    else
        return handle_valve_is_close();
}

prop_state_t ActionCycleOpenClose::handle_valve_is_open()
{
    // If we are at the end of the open period
    if (countdown.is_timer_zero())
    {
        // Then close the valve and start the timer for the close period
        PropulsionSystem.close_valve(*p_tank, cur_valve_index);
        countdown.set_timer_cc(get_ctrl_cycles_per_close_period());
    }
    return this_state;
}

prop_state_t ActionCycleOpenClose::handle_valve_is_close()
{
    // If we are at the end of the close period
    if (countdown.is_timer_zero())
    {
        // Determine if we have ran out of cycles
        if (cycle_count >= get_max_cycles())
            return handle_out_of_cycles();
        // Otherwise, start another open-close cycle
        start_cycle();
    }
    return this_state;
}

void ActionCycleOpenClose::start_cycle()
{
    cycle_count++;
    cur_valve_index = select_valve_index();
    DD("[*] Starting ActionCycleOpenClose cycle: %zu (opening valve %zu)\n",
       cycle_count, cur_valve_index);
    // Open the valve and start the timer for the open period
    PropulsionSystem.open_valve(*p_tank, cur_valve_index);
    countdown.set_timer_cc(controller->ctrl_cycles_per_filling_period.get());
}

// ------------------------------------------------------------------------
// PropState Pressurizing
// ------------------------------------------------------------------------

void PropState_Pressurizing::enter()
{
    DD("[*] ==> entered PropState_Pressurizing\n");
    ActionCycleOpenClose::enter();
}

bool PropState_Pressurizing::can_enter() const
{
    bool was_await_pressurizing = controller->check_current_state(prop_state_t::await_pressurizing);
    bool is_schedule_valid = controller->validate_schedule();
    // Allow from idle because sometimes we can immediately pressurize
    bool was_idle = controller->check_current_state(prop_state_t::idle);
    bool is_functional = PropulsionSystem.is_functional();

    // It is time to pressurize when we have min_cycles_needed - 1 cycles left
    bool is_time_to_pressurize =
        controller->cycles_until_firing.get() == controller->min_cycles_needed() - 1;
    
    return ((was_await_pressurizing || was_idle) && is_time_to_pressurize && is_schedule_valid && is_functional);
}

prop_state_t PropState_Pressurizing::evaluate()
{
    if (controller->can_enter_state(prop_state_t::handling_fault))
        return prop_state_t::handling_fault;
    return ActionCycleOpenClose::evaluate();
}

prop_state_t PropState_Pressurizing::handle_out_of_cycles()
{
    DD("[!] Pressurize Failed -- Out Of Cycles!\n");
    controller->pressurize_fail_fault_f.signal();
    // If the fault is not supressed, then go to handling_fault
    if (controller->pressurize_fail_fault_f.is_faulted())
    {
        DD("[!] --> transitioning to handling_fault\n");
        return prop_state_t::handling_fault;
    }
    // Fault is suppressed
    DD("[+] Underpressured fault suppressed --> transitioning to await_firing\n");
    return prop_state_t::await_firing;
}

// ------------------------------------------------------------------------
// PropState Await Firing
// ------------------------------------------------------------------------

bool PropState_AwaitFiring::can_enter() const
{
    bool was_pressurizing = controller->check_current_state(prop_state_t::pressurizing);
    bool is_functional = PropulsionSystem.is_functional();
    return controller->validate_schedule() && was_pressurizing && is_functional;
}

void PropState_AwaitFiring::enter()
{
    DD("[*] ==> entered PropState_AwaitFiring\n");
}

prop_state_t PropState_AwaitFiring::evaluate()
{
    if (controller->can_enter_state(prop_state_t::handling_fault))
        return prop_state_t::handling_fault;
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
    bool is_functional = PropulsionSystem.is_functional();
    return was_await_firing && is_time_to_fire && is_functional;
}

void PropState_Firing::enter()
{
    DD("[*] ==> entered PropState_Firing\n");
    PropulsionSystem.start_firing();
    controller->num_prop_firings_f.set(controller->num_prop_firings_f.get() + 1);
}

prop_state_t PropState_Firing::evaluate()
{
    if (controller->can_enter_state(prop_state_t::handling_fault))
    {
        PropulsionSystem.reset(); // Hard reset everything then go into handling_fault
        return prop_state_t::handling_fault;
    }

    DD("[-] ==> PropState_Firing is evaluating\n");
    if (is_schedule_empty())
    {
        PropulsionSystem.disable();
        DD("[+] ==> schedule is now empty\n");
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
    // Return true if any of the faults are actually faulted.
    //  This allows us to ignore bad sensors if the ground
    // decides to suppress/override certain faults
    bool b_pressurize_fail = controller->pressurize_fail_fault_f.is_faulted();
    bool b_overpressured = controller->overpressure_fault_f.is_faulted();
    bool b_tank2_temp = controller->tank2_temp_high_fault_f.is_faulted();
    bool b_tank1_temp = controller->tank1_temp_high_fault_f.is_faulted();
    return PropulsionSystem.is_functional() &&
            (b_pressurize_fail ||
            b_overpressured ||
            b_tank2_temp ||
            b_tank1_temp);
}

void PropState_HandlingFault::enter()
{
    DD("[*] ==> entered PropState_HandlingFault\n");
}

prop_state_t PropState_HandlingFault::evaluate()
{
    // If we can vent, then vent
    if (controller->can_enter_state(prop_state_t::venting))
    {
        DD("[*] Going into venting\n");
        return prop_state_t::venting;
    }
    // If faults are still signaled, then stay in HandlingFault
    if (can_enter())
    {
        return this_state;
    }
    // Otherwise, return to idle
    return prop_state_t::idle;
}
// ------------------------------------------------------------------------
// PropState Venting
// ------------------------------------------------------------------------

void PropState_Venting::enter()
{
    DD("[*] ==> entered PropState_Venting\n");
    if (determine_faulted_tank() == 2)
    {
        p_tank = &Tank2;
        cur_valve_index = saved_tank2_valve_choice;
    }
    else
    {
        p_tank = &Tank1;
    }
    ActionCycleOpenClose::enter();
}

bool PropState_Venting::can_enter() const
{
    bool b_overpressured = controller->overpressure_fault_f.is_faulted();
    bool b_tank2_temp = controller->tank2_temp_high_fault_f.is_faulted();
    bool b_tank1_temp = controller->tank1_temp_high_fault_f.is_faulted();
    return PropulsionSystem.is_functional() &&
           (b_overpressured ||
           b_tank2_temp ||
           b_tank1_temp);
}

unsigned int PropState_Venting::determine_faulted_tank()
{
    // If both tanks want to vent, then we determine who to vent by choosing
    // the tank that we did not vent the last time that we vented.
    if (tank1_wants_to_vent() && tank2_wants_to_vent())
    {
        if (tank_choice == 1)
        {
            return tank_choice = 2;
        }
        else
        {
            return tank_choice = 1;
        }
    }
    // Otherwise, only one of the tanks want to vent, so determine who that is.
    if (tank1_wants_to_vent())
    {
        return tank_choice = 1;
    }
    return tank_choice = 2;
}

bool PropState_Venting::has_succeeded() const
{
    if (tank_choice == 1)
        return !controller->tank1_temp_high_fault_f.is_faulted();
    // tank == Tank2
    return !controller->tank2_temp_high_fault_f.is_faulted() &&
           !controller->overpressure_fault_f.is_faulted();
}

size_t PropState_Venting::select_valve_index()
{
    if (tank_choice == 1)
        return controller->tank1_valve.get();
    // tank == Tank2
    // Want to cycle through the valves so that we don't
    return (saved_tank2_valve_choice++) % 4;
}

prop_state_t PropState_Venting::handle_out_of_cycles()
{
    DD("[-] PropState_Venting has finished venting\n");
    // If max_venting_cycles == 1, that means the PropFaultHandler has determined
    // previously that both tanks want to vent. We automatically transition to
    // handling_fault because we only vented for one cycle as oppose to 20.
    if (controller->max_venting_cycles.get() == 1)
        return prop_state_t::handling_fault;

    // Return disabled if the current tank is the only tank that wants to vent
    // and it is still faulted
    bool tank1_vented_but_tank2_is_faulted = tank_choice == 1 && tank2_wants_to_vent();
    bool tank2_vented_but_tank1_is_faulted = tank_choice == 2 && tank1_wants_to_vent();
    if (tank1_vented_but_tank2_is_faulted || tank2_vented_but_tank1_is_faulted)
        return prop_state_t::handling_fault;

    // Go into disabled if we believe that the we have failed (vented for 20
    // venting cycles and faults are still signaled)
    return prop_state_t::disabled;
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

void manual_eval(WritableStateField<unsigned int> &sched,
                 Devices::Tank &tank,
                 unsigned int valve_num)
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
