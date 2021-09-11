
#include "prop_shared.h"

void test_initialization()
{
    TestFixture tf;
    check_state(prop_state_t::disabled);
    TEST_ASSERT_FALSE(tf.pc->pressurize_fail_fault_f.is_faulted());
    TEST_ASSERT_FALSE(tf.pc->overpressure_fault_f.is_faulted());
    TEST_ASSERT_FALSE(tf.pc->tank2_temp_high_fault_f.is_faulted());
    TEST_ASSERT_FALSE(tf.pc->tank1_temp_high_fault_f.is_faulted());
}

// Test that PropController remains in the disabled state despite the existence of a valid schedule
void test_disable()
{
    TestFixture tf;
    // Prop should remain in disabled state until manually set to some other state
    tf.set_schedule(200, 300, 400, 500, tf.pc->min_cycles_needed());
    // Firing time is set 5 control cycles from now but we should not fire since
    // state is still disabled
    check_state(prop_state_t::disabled);
    for (size_t i = 0; i < tf.pc->min_cycles_needed(); ++i)
    {
        tf.step();
        check_state(prop_state_t::disabled);
    }
}

// Test that Prop ignores a schedule with fewer than prop_controller->min_cycles_needed() cycles
// Test that Prop ignores a schedule if a value exceeds 999
void test_illegal_schedule()
{
    TestFixture tf;
    tf.set_state(prop_state_t::idle);
    // Prop should ignore because fewer than min_cycles_needed()
    tf.set_schedule(200, 400, 800, 100, tf.pc->min_cycles_needed() - 1);
    tf.step();
    // State should remain in idle because 10001
    check_state(prop_state_t::idle);
    tf.set_schedule(200, 400, 800, 1001, tf.pc->min_cycles_needed());
    tf.step();
    // State should remain in idle because valve 4 is scheduled for 1000
    check_state(prop_state_t::idle);
}

// Test that a schedule with fire_cycle set to prop_controller->min_cycles_needed() and all valve
// schedules less than 1000 should be considered valid
// Test that we should go directly into pressurizing (as oppose to await_pressurizing)
void test_idle_to_pressurizing_helper(bool functional = true)
{
    TestFixture tf;
    PropulsionSystem.set_is_functional(functional);
    tf.set_state(prop_state_t::idle);
    tf.set_schedule(200, 400, 800, 999, tf.pc->min_cycles_needed());
    tf.step();
    // Go immediately into pressurizing
    if(functional)
        check_state(prop_state_t::pressurizing);
    else check_state(prop_state_t::idle);
}

void test_idle_to_pressurizing()
{
    test_idle_to_pressurizing_helper();
}

// Test that a schedule with fire_cycle set to tf.pc->min_cycles_needed() + 1 will cause
// Prop to enter await_pressurizing for 1 cycle before entering pressurizing
void test_idle_to_await_pressurize_helper(bool functional = true)
{
    TestFixture tf;
    PropulsionSystem.set_is_functional(functional);
    tf.set_state(prop_state_t::idle);
    tf.set_schedule(200, 400, 800, 100, tf.pc->min_cycles_needed() + 1);
    tf.step();
    // Go into await_pressurizing because we have more than enough time
    if(functional){
        check_state(prop_state_t::await_pressurizing);
        tf.step();
        check_state(prop_state_t::pressurizing);
    }
    else check_state(prop_state_t::idle);
    
}

void test_idle_to_await_pressurize()
{
    test_idle_to_await_pressurize_helper();
}

// Test that Prop waits in await_pressurizing if a schedule is set with fire_cycle exceeding
// tf.pc->min_cycles_needed()
void test_await_pressurize_to_pressurize_helper(bool functional = true)
{
    TestFixture tf;
    PropulsionSystem.set_is_functional(functional);
    tf.set_state(prop_state_t::idle);
    tf.set_schedule(200, 400, 12, 800, tf.pc->min_cycles_needed() + 10);
    tf.step();
    if(functional)
    {
        check_state(prop_state_t::await_pressurizing);
        tf.execute_until_state_change();
        check_state(prop_state_t::pressurizing);
    }
    else check_state(prop_state_t::idle);
}

void test_await_pressurize_to_pressurize()
{
    test_await_pressurize_to_pressurize_helper();
}

// Test that we pressurize for 19 cycles before transitioning to await_firing
void test_pressurize_to_await_firing_helper(bool functional = true)
{
    TestFixture tf;
    PropulsionSystem.set_is_functional(functional);
    tf.set_state(prop_state_t::idle);
    tf.set_schedule(200, 200, 200, 200, tf.pc->min_cycles_needed());
    tf.step();
    if(functional)
    {
        check_state(prop_state_t::pressurizing);

        tf.execute_step(do_nothing, 19 * tf.ctrl_cycles_per_pressurizing_cycle(), simulate_at_threshold);
        TEST_ASSERT_TRUE(Tank2.get_pressure() > tf.pc->threshold_firing_pressure.get());
        tf.step();
        TEST_ASSERT_TRUE(tf.pc->is_at_threshold_pressure());
        check_state(prop_state_t::await_firing);
    }
    else check_state(prop_state_t::idle);
}

void test_pressurize_to_await_firing()
{
    test_pressurize_to_await_firing_helper();
}

// Test the case where we pressurize for the maximum 20 pressurizing cycles. Make sure that we still enter await_firing
void test_pressurize_to_firing_helper(bool functional = true)
{
    // There is no going from pressurizing into firing
    TestFixture tf;
    PropulsionSystem.set_is_functional(functional);
    tf.set_state(prop_state_t::idle);
    tf.set_schedule(200, 400, 12, 800, tf.pc->min_cycles_needed());
    tf.step();
    if(functional)
    {
        check_state(prop_state_t::pressurizing);
        // At threshold at the end of the 20 pressurizing cycles
        tf.execute_step(do_nothing, 20 * tf.ctrl_cycles_per_pressurizing_cycle(), simulate_at_threshold);
        tf.step();
        check_state(prop_state_t::await_firing);
        tf.execute_until_state_change();
        check_state(prop_state_t::firing);
    }
    else check_state(prop_state_t::idle);
}

void test_pressurizing_to_idle_out_of_cycles(){
    TestFixture tf;
    PropulsionSystem.set_is_functional(true);
    tf.set_state(prop_state_t::idle);
    tf.set_schedule(200, 400, 12, 800, 2*tf.pc->min_cycles_needed());
    tf.step();


}

void test_pressurize_to_firing()
{
    test_pressurize_to_firing_helper();
}

// Test that when we are in pressurizing state, that Tank1 valve is opened and that Tank1 valve is closed when we
// enter await_firing
void test_pressurizing()
{
    TestFixture tf;
    tf.set_state(prop_state_t::idle);
    unsigned int cycles_until_fire = tf.pc->min_cycles_needed();
    tf.set_schedule(700, 200, 200, 800, cycles_until_fire);
    tf.step(); // transitioned from idle -> pressurizing
    check_state(prop_state_t::pressurizing);
    tf.step();                                // first cycle of pressurizing
    TEST_ASSERT_TRUE(Tank1.is_valve_open(0)); // Tank1 valve is opened
    simulate_at_threshold();
    tf.execute_until_state_change();
    TEST_ASSERT_FALSE(Tank1.is_valve_open(0)); // Tank1 valve is not opened
    check_state(prop_state_t::await_firing);
}

// Test the failure case where we are unable to reach threshold pressurize  in 20 cycles
void test_pressurize_fail()
{
    TestFixture tf;
    tf.set_state(prop_state_t::idle);
    tf.set_schedule(700, 200, 200, 800, tf.pc->min_cycles_needed() * 2);
    tf.step();                       // now in await_pressurizing
    tf.execute_until_state_change(); // now in pressurizing
    check_state(prop_state_t::pressurizing);

    // Same command here used in test_test_pressurize_to_firing but added 1 to the number of cycles
    tf.execute_step(do_nothing, 20 * tf.ctrl_cycles_per_pressurizing_cycle() + 1, simulate_at_threshold);
    tf.step();
    check_state(prop_state_t::handling_fault);
    tf.step();
    // Stay in handling_fault
    check_state(prop_state_t::handling_fault);
    assert_fault_state(true, pressurize_fail_fault_f);
    // If the ground suppresses this fault then return to idle
    tf.pc->pressurize_fail_fault_f.suppress_f.set(true);
    assert_fault_state(false, pressurize_fail_fault_f);
    tf.step();
    check_state(prop_state_t::idle);
}

void test_suppress_underpressure_fault()
{
    TestFixture tf;
    tf.set_state(prop_state_t::idle);
    tf.set_schedule(700, 200, 200, 800, 2 * tf.pc->min_cycles_needed());
    tf.pc->pressurize_fail_fault_f.suppress_f.set(true);
    tf.step();                       // enter await_pressurizing
    tf.execute_until_state_change(); // now in pressurizing
    check_state(prop_state_t::pressurizing);
    // Same setup as test_pressurize_fail
    tf.execute_step(do_nothing, 20 * tf.ctrl_cycles_per_pressurizing_cycle() + 1, simulate_at_threshold);
    tf.step();
    // But this time, we should not enter handling_fault
    assert_fault_state(false, pressurize_fail_fault_f); // assert not faulted
    // We should reach await_firing
    check_state(prop_state_t::await_firing);
}

// Test that when we have ran out of cycles and we are suppressed, we enter await_firing
void test_suppress_underpressure_fault_max_cycles()
{
    TestFixture tf;
    tf.set_state(prop_state_t::idle);
    // Suppress it early
    tf.pc->pressurize_fail_fault_f.suppress_f.set(true);
    tf.set_schedule(700, 200, 200, 800, 2 * tf.pc->min_cycles_needed());
    tf.step();                       // enter await_pressurizing
    tf.execute_until_state_change(); // now in pressurizing
    check_state(prop_state_t::pressurizing);
    tf.step(get_persistence(pressurize_fail_fault_f) + 1);
    assert_fault_state(false, pressurize_fail_fault_f); // assert not faulted
    tf.execute_until_state_change();
    // We should reach await_firing
    check_state(prop_state_t::await_firing);
}

// Test that when we are in await_firing, all valves are closed, the the schedule is valid
void test_await_firing()
{
    TestFixture tf;
    tf.set_state(prop_state_t::idle);
    unsigned int cycles_until_fire = tf.pc->min_cycles_needed() + 4;
    tf.set_schedule(700, 200, 200, 800, cycles_until_fire);
    tf.execute_step(do_nothing, 20 * tf.ctrl_cycles_per_pressurizing_cycle(), simulate_at_threshold);
    tf.step();
    check_state(prop_state_t::await_firing);
    // valves should be closed
    TEST_ASSERT_FALSE(Tank1.is_valve_open(0));
    TEST_ASSERT_FALSE(Tank2.is_valve_open(0));
    TEST_ASSERT_FALSE(Tank2.is_valve_open(1));
    TEST_ASSERT_FALSE(Tank2.is_valve_open(2));
    TEST_ASSERT_FALSE(Tank2.is_valve_open(3));
    // schedule should be valid
    TEST_ASSERT_TRUE(tf.pc->validate_schedule());
}

// Test that PropulsionSystem is_firing() returns True iff we are in the firing state
void test_firing()
{
    TestFixture tf;
    tf.set_state(prop_state_t::idle);
    TEST_ASSERT_FALSE(PropulsionSystem.is_firing());
    tf.set_schedule(700, 200, 200, 800, tf.pc->min_cycles_needed());
    simulate_at_threshold();
    tf.step(tf.pc->min_cycles_needed());
    check_state(prop_state_t::firing);
    TEST_ASSERT_TRUE(PropulsionSystem.is_firing())
    unsigned int expected_firing_cycles = (800 / PAN::control_cycle_time_ms) + 1; // roundup
    TEST_ASSERT_EQUAL(expected_firing_cycles, tf.execute_until_state_change());
    TEST_ASSERT_FALSE(PropulsionSystem.is_firing());
    // can't really check valve states unless we want to make a fake interval timer
}

// Test that if we order Prop to fire at tf.pc->min_cycles_needed()
// then Prop is definitely firing after this amount of cycles
void test_firing_to_idle_helper(bool functional = true)
{
    TestFixture tf;
    PropulsionSystem.set_is_functional(functional);
    tf.set_state(prop_state_t::idle);
    tf.set_schedule(700, 200, 200, 800, tf.pc->min_cycles_needed());
    simulate_at_threshold();
    // [cycles_until_fire] from now, this had better be firing.
    tf.step(tf.pc->min_cycles_needed() - 1);
    if(functional)
    {
        check_state(prop_state_t::await_firing);
        tf.step();
        check_state(prop_state_t::firing);
        // Test that PropulsionSystem is firing while we are in the firing state
        // 800 is the biggest value
        unsigned int cycles_firing = (800 / PAN::control_cycle_time_ms) + 1; // round up
        for (size_t i = 0; i < cycles_firing; ++i)
        {
            check_state(prop_state_t::firing);
            TEST_ASSERT_TRUE(PropulsionSystem.is_firing())
            tf.step();
        }
        // On the next control cycle, we should be back to idle
        TEST_ASSERT_FALSE(PropulsionSystem.is_firing())
        check_state(prop_state_t::idle);
    }
    else check_state(prop_state_t::idle);
}

void test_firing_to_idle()
{
    test_firing_to_idle_helper();
}

// Test that we use the backup valve when it is requested
void test_use_backup()
{
    TestFixture tf;
    tf.simulate_pressurizing();
    tf.step(3);
    TEST_ASSERT_TRUE(Tank1.is_valve_open(0));
    TEST_ASSERT_FALSE(Tank2.is_valve_open(1));
    tf.set_state(prop_state_t::disabled);
    tf.step();
    // Set to disabled
    check_state(prop_state_t::disabled);
    //  Both valves should be closed
    TEST_ASSERT_FALSE(Tank1.is_valve_open(1));
    TEST_ASSERT_FALSE(Tank2.is_valve_open(0));
    tf.pc->tank1_valve.set(1); // set the valve to the backup valve
                               // Go back to pressurizing
    tf.simulate_pressurizing();
    check_state(prop_state_t::pressurizing);
    tf.step();
    TEST_ASSERT_TRUE(Tank1.is_valve_open(1));
    TEST_ASSERT_FALSE(Tank2.is_valve_open(0));
}

// Test the tank2 venting response
void test_vent_outer_tank()
{
    TestFixture tf;
    tf.simulate_pressurizing();
    tf.step(4);
    simulate_overpressured();
    tf.step(get_persistence(overpressure_fault_f) + 2);
    assert_fault_state(true, overpressure_fault_f);
    check_state(prop_state_t::handling_fault);
    tf.step();
    // Make sure that this thing cycles
    check_state(prop_state_t::venting);
    assert_fault_state(true, overpressure_fault_f);
    tf.step();
    for (size_t i = 0; i < 4; ++i)
    {
        TEST_ASSERT_TRUE(Tank2.is_valve_open(i));
        tf.step(tf.pc->ctrl_cycles_per_filling_period.get());
        // All valves should be closed
        TEST_ASSERT_FALSE(Tank2.is_valve_open(0));
        TEST_ASSERT_FALSE(Tank2.is_valve_open(1));
        TEST_ASSERT_FALSE(Tank2.is_valve_open(2));
        TEST_ASSERT_FALSE(Tank2.is_valve_open(3));
        tf.step(tf.pc->ctrl_cycles_per_close_period.get());
    }
}

// Test the tank1 venting response
void test_vent_inner_tank()
{
    TestFixture tf;
    tf.simulate_firing();
    tf.step(1);
    simulate_tank1_high();
    // +2 because the fault is faulted after persistence + 1
    // And it takes 1 state to transition to handling_fault
    tf.step(get_persistence(tank1_temp_high_fault_f) + 2);
    check_state(prop_state_t::handling_fault);
    assert_fault_state(true, tank1_temp_high_fault_f);
    tf.step(2);
    assert_fault_state(true, tank1_temp_high_fault_f);
    check_state(prop_state_t::venting);
    TEST_ASSERT_TRUE(Tank1.is_valve_open(0));
}

void test_vent_both_tanks()
{
    TestFixture tf;
    tf.simulate_pressurizing();
    simulate_overpressured();
    simulate_tank1_high();
    tf.step(get_persistence(overpressure_fault_f) + 2);
    assert_fault_state(true, tank1_temp_high_fault_f);
    assert_fault_state(true, overpressure_fault_f);
    check_state(prop_state_t::handling_fault);
    tf.step();
    check_state(prop_state_t::venting);
    while(tf.pc->prop_state_f.get() == static_cast<unsigned int>(prop_state_t::venting))
    {
        tf.step();
    }
    check_state(prop_state_t::handling_fault);
}

//test state changes don't happen if not functional
void test_non_functional_transitions()
{
    test_idle_to_pressurizing_helper(false);
    test_idle_to_await_pressurize_helper(false);
    test_await_pressurize_to_pressurize_helper(false);
    test_pressurize_to_await_firing_helper(false);
    test_pressurize_to_firing_helper(false);
    test_firing_to_idle_helper(false);
}


// These two tests are manually checked, so the for loop is conditionally compiled
// in order to not lag everything

void test_temp_sensor_logic()
{
#ifdef _PRINT_SENSORS
    for (unsigned int i = 0; i < 1023; i++)
    {
        Tank1.fake_tank1_temp_sensor_read = i;
        std::printf("Tank1 temp read %u --> %d\n", i, Tank1.get_temp());
    }
#endif
}

void test_pressure_sensor_logic()
{
#ifdef _PRINT_SENSORS
    // analogRead range for high gain: 20 - 1018
    // analogRead range for low gain 12 - 1011
    unsigned int thresh = Tank2.amp_threshold;
    unsigned int high_read_min = 20;
    unsigned int high_read_max = 1018;
    unsigned int low_read_min = 12;
    unsigned int low_read_max = 1011;

    for (unsigned int high = high_read_min; high < high_read_max; high++)
    {
        Tank2.fake_tank2_pressure_high_read = high;
        if (high > thresh)
            for (unsigned int low = low_read_min; low < low_read_max; low++)
            {
                Tank2.fake_tank2_pressure_low_read = low;
                std::printf("Tank2 pressure: high ignored (>%d), low %u --> %f\n", thresh, low, Tank2.get_pressure());
            }
        else
        {
            Tank2.fake_tank2_pressure_low_read = 0;
            std::printf("Tank2 pressure: high %u, low ignored --> %f\n", high, Tank2.get_pressure());
        }
    }
#endif
}
int test_prop_controller()
{
    // generated the following with:
    // cat test/test_fsw_prop_controller//test_prop_controller.cpp | grep "void test_" | sed 's/^void \(.*\)$/\1/' | sed 's/()/);/g'| sed -e 's/^/RUN_TEST(/'
    UNITY_BEGIN();
    RUN_TEST(test_initialization);
    RUN_TEST(test_disable);
    RUN_TEST(test_non_functional_transitions);
    RUN_TEST(test_illegal_schedule);
    RUN_TEST(test_idle_to_pressurizing);
    RUN_TEST(test_idle_to_await_pressurize);
    RUN_TEST(test_await_pressurize_to_pressurize);
    RUN_TEST(test_pressurize_to_await_firing);
    RUN_TEST(test_pressurizing_to_idle_out_of_cycles);
    RUN_TEST(test_pressurize_to_firing);
    RUN_TEST(test_pressurizing);
    RUN_TEST(test_pressurize_fail);
    RUN_TEST(test_suppress_underpressure_fault);
    RUN_TEST(test_suppress_underpressure_fault_max_cycles);
    RUN_TEST(test_await_firing);
    RUN_TEST(test_firing);
    RUN_TEST(test_firing_to_idle);
    RUN_TEST(test_use_backup);
    RUN_TEST(test_vent_outer_tank);
    RUN_TEST(test_vent_inner_tank);
    RUN_TEST(test_vent_both_tanks);
    RUN_TEST(test_temp_sensor_logic);
    RUN_TEST(test_pressure_sensor_logic);
    return UNITY_END();
}

#ifdef DESKTOP
int main()
{
    return test_prop_controller();
}
#else
#include <Arduino.h>
void setup()
{
    delay(2000);
    Serial.begin(9600);
    test_prop_controller();
}

void loop() {}
#endif
