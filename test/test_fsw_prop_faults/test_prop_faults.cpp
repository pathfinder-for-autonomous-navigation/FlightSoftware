//
// Created by athena on 4/21/20.
//

#include "../test_fsw_prop_controller/prop_shared.h"
unsigned int one_day_ccno = PAN::one_day_ccno;
unsigned int &cc_count = TimedControlTaskBase::control_cycle_count;

void test_respect_disabled()
{
    // If faults are signalled, but we are in disable, then we should not be handling faults are detecting faults
    TestFixture tf;
    simulate_overpressured(); // we are about to blow up...
    // signal the fault to persistence times then detect it on the 11th
    tf.step(get_persistence(overpressure_fault_f) + 1);
    assert_fault_state(true, overpressure_fault_f);
    // But we should not enter any fault handling state because we disabled
    for (size_t i = 0; i < 20; ++i)
    {
        tf.step();
        check_state(prop_state_t::disabled);
        TEST_ASSERT_EQUAL(tf.pfh->execute(), fault_response_t::none);
    }
}

// Test that underpressure event is detected and causes us to enter standby
void test_underpressured_detect()
{
    TestFixture tf;
    tf.simulate_underpressured();
    tf.step(get_persistence(pressurize_fail_fault_f) + 1);
    assert_fault_state(true, pressurize_fail_fault_f);
}

void test_overpressured_detect()
{
    TestFixture tf;
    simulate_overpressured();
    tf.step(get_persistence(overpressure_fault_f) + 1);
    assert_fault_state(true, overpressure_fault_f);
}

void test_tank1_temp_high_detect()
{
    TestFixture tf;
    simulate_tank1_high();
    tf.step(get_persistence(tank1_temp_high_fault_f) + 1);
    assert_fault_state(true, tank1_temp_high_fault_f);
}

void test_tank2_temp_high_detect()
{
    TestFixture tf;
    simulate_tank2_high();
    tf.step(get_persistence(tank2_temp_high_fault_f) + 1);
    assert_fault_state(true, tank2_temp_high_fault_f);
}

void test_both_tanks_high_detect()
{
    TestFixture tf;
    // Test that when multiple fault events occur, both faults are detected
    simulate_tank2_high();
    tf.step(get_persistence(tank2_temp_high_fault_f) + 1);
    assert_fault_state(true, tank2_temp_high_fault_f);
    simulate_tank1_high();
    tf.step(get_persistence(tank1_temp_high_fault_f) + 1);
    // Both faults should be signalled
    assert_fault_state(true, tank1_temp_high_fault_f);
    assert_fault_state(true, tank2_temp_high_fault_f);
}

void test_multiple_faults_detect()
{
    TestFixture tf;
    // Test that when multiple fault events occur, both faults are detected
    simulate_overpressured();
    tf.step(get_persistence(overpressure_fault_f));
    simulate_tank1_high();
    // Tank1 high occurs before the overpressure_fault is faulted
    tf.step();
    assert_fault_state(true, overpressure_fault_f);
    // Tank1 temp should have been detected on the cycle at which overpressure is faulted
    tf.step(get_persistence(tank1_temp_high_fault_f));
    // Both faults should be signalled
    assert_fault_state(true, overpressure_fault_f);
    assert_fault_state(true, tank1_temp_high_fault_f);
    simulate_tank2_high();
    tf.step(get_persistence(tank2_temp_high_fault_f) + 1);
    // All three should be signalled
    assert_fault_state(true, overpressure_fault_f);
    assert_fault_state(true, tank1_temp_high_fault_f);
    assert_fault_state(true, tank2_temp_high_fault_f);
}

void run_fault_detection_tests()
{
    RUN_TEST(test_underpressured_detect);
    RUN_TEST(test_overpressured_detect);
    RUN_TEST(test_tank1_temp_high_detect);
    RUN_TEST(test_tank2_temp_high_detect);
    RUN_TEST(test_both_tanks_high_detect);
    RUN_TEST(test_multiple_faults_detect);
}

void test_underpressured_response()
{
    TestFixture tf;
    // In the event that Prop fails to pressurize
    tf.simulate_underpressured();
    // check that pressurize_fail_fault_f is faulted immediately and state enters handling_fault
    assert_fault_state(true, pressurize_fail_fault_f);
    check_state(prop_state_t::handling_fault);
    // check that further executions of PropFaultHandler should still suggest standby response
    for (size_t i = 0; i < 31; ++i)
    {
        tf.step();
        assert_fault_state(true, pressurize_fail_fault_f);
        check_state(prop_state_t::handling_fault);
        TEST_ASSERT_EQUAL(tf.pfh->execute(), fault_response_t::standby);
    }
    // If the ground suppresses the fault
    suppress_fault(pressurize_fail_fault_f);
    tf.step();
    // Check that the fault is no longer faulted
    assert_fault_state(false, pressurize_fail_fault_f);
    // Check that the state returns to idle
    check_state(prop_state_t::idle);
    // Check that the fault remains suppressed even though we continue to detect the fault event
    tf.simulate_underpressured();
    assert_fault_state(false, pressurize_fail_fault_f);
    // We should now be in firing since simulate_underpressured() executes the full
    // number of cycles_until_firing
    check_state(prop_state_t::firing);
}

void test_overpressured_response()
{
    TestFixture tf;
    tf.simulate_pressurizing();
    // tf.step for some random number of pressurizing cycles so that we
    //  are in the middle of pressurizing
    tf.step(2 * tf.ctrl_cycles_per_pressurizing_cycle() + 1);
    // Check that the tank1 valve should be opened since we are pressurizing
    TEST_ASSERT_TRUE(Tank1.is_valve_open(0));

    simulate_overpressured();
    tf.step(get_persistence(overpressure_fault_f) + 2);
    check_state(prop_state_t::handling_fault);
    TEST_ASSERT_FALSE(Tank1.is_valve_open(0)); // Tank1 valve should be closed immediately
    tf.step();
    check_state(prop_state_t::venting); // Check that we have entered the venting state
    tf.step();
    // Tank2 should open valves and vent until pressure is below max safe pressure
    TEST_ASSERT_TRUE(Tank2.is_valve_open(0));
    // Test that we go into disabled when we have failed too many times
    tf.execute_until_state_change();
    check_state(prop_state_t::disabled);
    simulate_ambient();
    tf.step();
    assert_fault_state(false, overpressure_fault_f);
    // Since we are in disabled, it does not matter that the fault is no longer
    // a problem, we must remain in disabled
    check_state(prop_state_t::disabled);
    tf.step(50);
    check_state(prop_state_t::disabled);
}

// Test that we can detect high temp event while firing and respond accordingly
void test_tank1_temp_high_response()
{
    TestFixture tf;
    tf.simulate_firing();
    tf.step(2); // start firing a little bit
    simulate_tank1_high();
    tf.step(get_persistence(tank1_temp_high_fault_f) + 2);
    check_state(prop_state_t::handling_fault);
    tf.step();
    check_state(prop_state_t::venting);
    tf.step();
    // We should be venting Tank1
    TEST_ASSERT_TRUE(Tank1.is_valve_open(0));
    // Just for fun, suppose we decide to change to the backup valve
    tf.pc->tank1_valve.set(1);
    tf.step(tf.ctrl_cycles_per_pressurizing_cycle());
    TEST_ASSERT_TRUE(Tank1.is_valve_open(1));
    assert_fault_state(true, tank1_temp_high_fault_f);
    tf.step(3);
    // Now pretend that venting did fix the issue, the fault should be unsignalled
    simulate_ambient();
    tf.step();
    assert_fault_state(false, tank1_temp_high_fault_f);
    tf.step();
    check_state(prop_state_t::idle); // We should go into idle
}

void test_tank2_temp_high_response()
{
    TestFixture tf;
    tf.set_state(prop_state_t::idle);
    tf.step(3);
    simulate_tank2_high();
    tf.step(get_persistence(tank2_temp_high_fault_f) + 2);
    check_state(prop_state_t::handling_fault);
    tf.step();
    check_state(prop_state_t::venting);
    tf.step();
    TEST_ASSERT_TRUE(Tank2.is_valve_open(0));
    tf.step(tf.pc->ctrl_cycles_per_cooling_period.get() +
            tf.pc->ctrl_cycles_per_filling_period.get());
    assert_fault_state(true, tank2_temp_high_fault_f);
    // Make sure this thing opens the next valve
    TEST_ASSERT_TRUE(Tank2.is_valve_open(1));
    simulate_ambient();
    tf.step();
    assert_fault_state(false, tank2_temp_high_fault_f);
    tf.step();
    check_state(prop_state_t::idle);
}

void test_tank2temphigh_undepressured_response()
{
    TestFixture tf;
    // Overpressured and High temp faults take precedence
    tf.simulate_underpressured();
    simulate_tank2_high();
    tf.step();
    check_state(prop_state_t::handling_fault);
    assert_fault_state(true, pressurize_fail_fault_f);
    tf.step(get_persistence(tank2_temp_high_fault_f) + 1);
    assert_fault_state(true, tank2_temp_high_fault_f);
    tf.step();
    check_state(prop_state_t::venting);
    tf.step();
    TEST_ASSERT_TRUE(Tank2.is_valve_open(0));
}

// Tank1 is venting but Tank2 is suddenly faulted
void test_tank2_fault_while_tank1_vent()
{
    TestFixture tf;
    // Save the cooling period and the max_pressurizing cycles for testing purpoes
    auto saved_cooling = tf.pc->ctrl_cycles_per_cooling_period.get();
    auto saved_max_cycles = tf.pc->max_pressurizing_cycles.get();

    tf.simulate_await_firing();
    // Simulate Tank1 high fault and detect it
    simulate_tank1_high();
    tf.step(get_persistence(tank1_temp_high_fault_f) + 2);
    check_state(prop_state_t::handling_fault);
    tf.step();
    check_state(prop_state_t::venting);
    // We are in the middle of venting Tank1
    tf.step(tf.pc->ctrl_cycles_per_filling_period.get() / 2 + 1);
    TEST_ASSERT_TRUE(Tank1.is_valve_open(0));

    // Now simulate Tank2 temperature high event and step until faulted
    simulate_tank2_high();
    tf.step(get_persistence(tank2_temp_high_fault_f) + 1);
    // Both should now be faulted
    assert_fault_state(true, tank2_temp_high_fault_f);
    assert_fault_state(true, tank1_temp_high_fault_f);
    tf.step();
    // PropFaultHandler should shorten max cycles to 1
    TEST_ASSERT_EQUAL(1, tf.pc->max_pressurizing_cycles.get());
    // PropFaultHandler change cooling period := filling period
    TEST_ASSERT_EQUAL(tf.pc->ctrl_cycles_per_filling_period.get(),
                      tf.pc->ctrl_cycles_per_cooling_period.get());
    tf.step();
    // Since the max_pressurizing_cycle is only 1, Prop should go into handling_fault
    check_state(prop_state_t::handling_fault);
    tf.step();
    check_state(prop_state_t::venting);
    tf.step();
    // We should now be venting Tank2 for one pressurizing cycle
    TEST_ASSERT_TRUE(Tank2.is_valve_open(0));
    tf.step(tf.pc->ctrl_cycles_per_filling_period.get());
    TEST_ASSERT_FALSE(Tank2.is_valve_open(0));
    tf.step(tf.pc->ctrl_cycles_per_cooling_period.get());

    // Then enter handling fault and get ready to vent Tank1
    check_state(prop_state_t::handling_fault);
    tf.step();
    check_state(prop_state_t::venting);
    tf.step();
    TEST_ASSERT_TRUE(Tank1.is_valve_open(0));
    tf.step(tf.pc->ctrl_cycles_per_filling_period.get());
    TEST_ASSERT_FALSE(Tank1.is_valve_open(0));
    tf.step(tf.pc->ctrl_cycles_per_cooling_period.get());

    // Then go back to handling_fault and get ready to vent Tank2
    check_state(prop_state_t::handling_fault);
    tf.step();
    check_state(prop_state_t::venting);
    tf.step();
    // Make sure that we use the next Tank2 valve so as to not veer off into some direction
    TEST_ASSERT_TRUE(Tank2.is_valve_open(1));

    // While Tank2 valve is opened... suppose Tank1 is no longer faulted
    tf.step(tf.pc->ctrl_cycles_per_filling_period.get() / 2 + 1);
    simulate_ambient();
    simulate_tank2_high();
    tf.step();
    assert_fault_state(false, tank1_temp_high_fault_f);
    assert_fault_state(true, tank2_temp_high_fault_f);
    // Upon seeing that both Tank faults are not faulted, PropFaultHandler
    //      should revert the cooling and the max_pressurizing_cycles
    TEST_ASSERT_EQUAL(saved_max_cycles, tf.pc->max_pressurizing_cycles.get());
    TEST_ASSERT_EQUAL(saved_cooling, tf.pc->ctrl_cycles_per_cooling_period.get());
    tf.step();

    // We should still be venting Tank2
    check_state(prop_state_t::venting);
    // That same valve should still be opened
    TEST_ASSERT_TRUE(Tank2.is_valve_open(1));
}

// All the sensors are broken and report fault
void test_all_faulted_sensors_broken_respect_disabled()
{
    TestFixture tf;

    // Prop should do the procedure described in the test above.

    // Suppose that venting fixes Tank2 temp high but not overpressured

    // Prop should continue taking turns venting

    // PropFaultHandler will detect this anomally and see that it is wasting
    // power

    // PropFaultHandler will order Prop to enter disabled
}

void run_fault_response_tests()
{
    RUN_TEST(test_underpressured_response);
    RUN_TEST(test_overpressured_response);
    RUN_TEST(test_tank1_temp_high_response);
    RUN_TEST(test_tank2_temp_high_response);
    RUN_TEST(test_tank2temphigh_undepressured_response);
    RUN_TEST(test_tank2_fault_while_tank1_vent);
    RUN_TEST(test_all_faulted_sensors_broken_respect_disabled);
}

// When we conflicting faults are signalled
void run_conflicting_fault_response_tests()
{
    // underpressure and overpressure both signalled

    // underpressured and tank2 temp high

    // underpressured and tank1 temp high
}

#ifdef DESKTOP
int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_respect_disabled);
    run_fault_detection_tests();
    run_fault_response_tests();
    return UNITY_END();
}
#else
#include <Arduino.h>
void setup()
{
    delay(2000);
    Serial.begin(9600);
}

void loop() {}
#endif
