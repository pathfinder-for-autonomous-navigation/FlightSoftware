//
// Created by athena on 4/21/20.
//

#include "../test_fsw_prop_controller/prop_shared.h"
unsigned int one_day_ccno = PAN::one_day_ccno;
unsigned int& cc_count = TimedControlTaskBase::control_cycle_count;


void test_respsect_disable_mode() {
    // If faults are signalled, but we are in disable, then we should not be handling faults are detecting faults
}

void test_underpressured_detect() {
    TestFixture tf;
    tf.simulate_underpressured();

    assert_fault_state(true, pressurize_fail_fault_f);
    check_state(prop_state_t::disabled);
    // Further executions of PropFaultHandler should still suggest standby response
    TEST_ASSERT_EQUAL(tf.pfh->execute(), fault_response_t::standby);
}

void test_overpressured_detect() {
    TestFixture tf;
    tf.simulate_overpressured();

    TEST_ASSERT_TRUE(tf.pc->overpressure_fault_f.is_faulted());
}

void test_tank1_temp_high_detect(){
    TestFixture tf;
    tf.simulate_tank1_high();

    TEST_ASSERT_TRUE(tf.pc->tank1_temp_high_fault_f.is_faulted());
}

void test_tank2_temp_high_detect(){
    TestFixture tf;
    tf.simulate_tank2_high();

    TEST_ASSERT_TRUE(tf.pc->tank2_temp_high_fault_f.is_faulted());
}

void test_both_tanks_high_detect() {
    TestFixture tf;
    // Test that when multiple fault events occur, both faults are detected
    tf.simulate_tank2_high();
    tf.simulate_tank1_high();
    TEST_ASSERT_TRUE(tf.pc->tank2_temp_high_fault_f.is_faulted());
    TEST_ASSERT_TRUE(tf.pc->tank1_temp_high_fault_f.is_faulted());
}

// both tank temperatures high and overpressured
void test_multiple_faults_detect() {
    TestFixture tf;
    // Test that when multiple fault events occur, both faults are detected
    tf.simulate_overpressured();
    tf.simulate_tank1_high();
    TEST_ASSERT_TRUE(tf.pc->overpressure_fault_f.is_faulted());
    TEST_ASSERT_TRUE(tf.pc->tank1_temp_high_fault_f.is_faulted());
    TEST_ASSERT_FALSE(tf.pc->tank2_temp_high_fault_f.is_faulted());
}


void run_fault_detection_tests(){
    RUN_TEST(test_underpressured_detect);
    RUN_TEST(test_overpressured_detect);
    RUN_TEST(test_tank1_temp_high_detect);
    RUN_TEST(test_tank2_temp_high_detect);
    RUN_TEST(test_both_tanks_high_detect);
    RUN_TEST(test_multiple_faults_detect);
}

void test_underpressured_response() {
    TestFixture tf;
    tf.simulate_underpressured();
}

void test_overpressured_response() {
    TestFixture tf;
    tf.simulate_overpressured();
}

void test_tank1_temp_high_response(){
    TestFixture tf;
    tf.simulate_tank1_high();
}

void test_tank2_temp_high_response(){
    TestFixture tf;
    tf.simulate_tank2_high();
}

void test_tank2temphigh_undepressured_response(){
    TestFixture tf;
    // Test that when multiple fault events occur, we will always trust pressure over temperature
    tf.simulate_underpressured();
    tf.simulate_tank2_high();
}

void run_fault_response_tests(){
    RUN_TEST(test_underpressured_response);
    RUN_TEST(test_overpressured_response);
    RUN_TEST(test_tank1_temp_high_response);
    RUN_TEST(test_tank2_temp_high_response);
    RUN_TEST(test_tank2temphigh_undepressured_response);
}

// When we conflicting faults are signalled
void run_conflicting_fault_response_tests(){
    // underpressure and overpressure both signalled

    // underpressured and tank2 temp high

    // underpressured and tank1 temp high
}

#ifdef DESKTOP
int main() {
    UNITY_BEGIN();
    RUN_TEST(test_respsect_disable_mode);
    run_fault_detection_tests();
    run_fault_response_tests();
    return UNITY_END();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_prop_faults();
}

void loop() {}
#endif
