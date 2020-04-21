//
// Created by athena on 4/21/20.
//

#include <unity.h>
#include "../StateFieldRegistryMock.hpp"
#include <fsw/FCCode/PropController.hpp>
#include <fsw/FCCode/PropFaultHandler.h>
#include <fsw/FCCode/prop_state_t.enum>

unsigned int one_day_ccno = PAN::one_day_ccno;
unsigned int& cc_count = TimedControlTaskBase::control_cycle_count;

class TestFixture {
public:
    StateFieldRegistryMock registry;

    std::shared_ptr<WritableStateField<unsigned int>> prop_state_fp;

    std::unique_ptr<PropController> pc;
    std::unique_ptr<PropFaultHandler> pfh;

    TestFixture() {
        pc = std::make_unique<PropController>(registry, 0);
        pfh = std::make_unique<PropFaultHandler>(registry);
    }

    void simulate_underpressured()
    {

    }
    void simulate_overpressured()
    {

    }
    void simulate_tank1_high()
    {

    }
    void simulate_tank2_high()
    {

    }

    ~TestFixture(){
        // Reset the prop between tests
        PropulsionSystem.reset();
    }
};

void test_respsect_disable_mode() {
    // If faults are signalled, but we are in disable, then we should not be handling faults are detecting faults
}

void test_underpressured_detect() {
    TestFixture tf;
    tf.simulate_underpressured();

    TEST_ASSERT_TRUE(tf.pc->pressurize_fail_fault_f.is_faulted());
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

void test_tank1temphigh_undepressured_response(){
    TestFixture tf;
    // Test that when multiple fault events occur, we prioritize faults
    tf.simulate_underpressured();
    tf.simulate_tank1_high();
}

void run_fault_response_tests(){
    RUN_TEST(test_underpressured_response);
    RUN_TEST(test_overpressured_response);
    RUN_TEST(test_tank1_temp_high_response);
    RUN_TEST(test_tank2_temp_high_response);
    RUN_TEST(test_tank1temphigh_undepressured_response);
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
