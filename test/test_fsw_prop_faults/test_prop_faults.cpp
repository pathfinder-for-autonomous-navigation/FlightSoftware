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
    tf.step(2);
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
    assert_fault_state(true, pressurize_fail_fault_f);
}

void test_overpressured_detect()
{
    TestFixture tf;
    simulate_overpressured();
    tf.step();
    assert_fault_state(true, overpressure_fault_f);
}

void test_tank1_temp_high_detect()
{
    TestFixture tf;
    simulate_tank1_high();
    tf.step();
    assert_fault_state(true, tank1_temp_high_fault_f);
}

void test_tank2_temp_high_detect()
{
    TestFixture tf;
    simulate_tank2_high();
    tf.step();
    assert_fault_state(true, tank2_temp_high_fault_f);
}

void test_both_tanks_high_detect()
{
    TestFixture tf;
    // Test that when multiple fault events occur, both faults are detected
    simulate_tank2_high();
    simulate_tank1_high();
    tf.step();
    assert_fault_state(true, tank2_temp_high_fault_f);
    assert_fault_state(true, tank1_temp_high_fault_f);
}

void test_multiple_faults_detect()
{
    TestFixture tf;
    // Test that when multiple fault events occur, both faults are detected
    simulate_overpressured();
    simulate_tank1_high();
    tf.step();
    assert_fault_state(true, overpressure_fault_f);
    assert_fault_state(true, tank1_temp_high_fault_f);
    assert_fault_state(false, tank2_temp_high_fault_f);
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
    // We should now enter await_firing and depend on the temperature sensor instead
    check_state(prop_state_t::await_firing);
}

void test_overpressured_response()
{
    TestFixture tf;
    tf.simulate_pressurizing();
    // tf.step for some random number of pressurizing cycles so that we are in the middle of pressurizing
    tf.step(2 * tf.ctrl_cycles_per_pressurizing_cycle() + 1);
    // Check that the tank1 valve should be opened since we are pressurizing
    TEST_ASSERT_TRUE(Tank1.is_valve_open(0));

    simulate_overpressured();
    // it takes us two control cycles to realize that we overpressured because sensors checked at the end of the execution cycle
    tf.step(2);
    check_state(prop_state_t::handling_fault);
    TEST_ASSERT_FALSE(Tank1.is_valve_open(0)); // Tank1 valve should be closed immediately
    // TODO
    tf.step();
    check_state(prop_state_t::venting); // Check that we have entered the venting state
    // Tank2 should open valves and vent until pressure is below max safe pressure
}

// Test that we can detect high temp event while firing and respond accordingly
void test_tank1_temp_high_response()
{
    TestFixture tf;
    tf.simulate_firing();
    tf.step(2); // start firing a little bit
    simulate_tank1_high();
    tf.step();
    check_state(prop_state_t::handling_fault);
    tf.step();
    // TODO: what do?
    check_state(prop_state_t::venting);
}

void test_tank2_temp_high_response()
{
    TestFixture tf;
    tf.set_state(prop_state_t::idle);
    tf.step(3);
    simulate_tank2_high();
    tf.step();
    check_state(prop_state_t::handling_fault);
    tf.step();
    check_state(prop_state_t::venting);
    // we should be opening the valves on tank2
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
    assert_fault_state(true, tank2_temp_high_fault_f);
    tf.step();
    // We should be about to open the valves on tank2
    check_state(prop_state_t::await_firing);
    tf.check_schedule(1000, 1000, 1000, 1000, 2);
}

void run_fault_response_tests()
{
    RUN_TEST(test_underpressured_response);
    RUN_TEST(test_overpressured_response);
    RUN_TEST(test_tank1_temp_high_response);
    RUN_TEST(test_tank2_temp_high_response);
    RUN_TEST(test_tank2temphigh_undepressured_response);
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
