#include "../StateFieldRegistryMock.hpp"

#include <fsw/FCCode/Fault.hpp>
#include <unity.h>

/**
 * @brief Test normal behavior of fault without overrides or supressings.
 */
void test_fault_normal_behavior() {
    unsigned int control_cycle_count = 0;

    // Test constructor
    Fault fault("fault", 5, control_cycle_count);

    // Test that adding the fault to the registry works
    StateFieldRegistry r;
    TEST_ASSERT(fault.add_to_registry(r));
    TEST_ASSERT(r.find_writable_field("fault"));
    TEST_ASSERT(r.find_writable_field("fault.override"));
    TEST_ASSERT(r.find_writable_field("fault.suppress"));
    TEST_ASSERT(r.find_writable_field("fault.unsignal"));

    Fault* fault_fp = static_cast<Fault*>(r.find_writable_field("fault"));

    TEST_ASSERT_FALSE(fault_fp->is_faulted());

    // Fault should not be signaled when signal condition happens under the persistence
    for(int i = 0; i < 4; i++) {
        control_cycle_count++; fault.signal();
        TEST_ASSERT_FALSE(fault_fp->is_faulted());
    }

    // Resetting the fault should not cause the fault to be signaled, but should cause
    // the num_conseuctive_faults to reset. Resultantly, incrementing the control
    // cycle count and signaling should not cause the fault to be signaled.
    fault.unsignal();
    control_cycle_count++; fault.signal();
    TEST_ASSERT_FALSE(fault_fp->is_faulted());

    // Keep signaling the fault until we're just underneath the fault's persistence
    // threshold.
    for(int i = 0; i < 4; i++) { control_cycle_count++; fault.signal(); }
    // Signaling the fault again should have no effect if the control cycle count
    // is not also incremented.
    fault.signal();
    TEST_ASSERT_FALSE(fault_fp->is_faulted());
    // Incrementing the control cycle count should now cause the fault to be signaled.
    control_cycle_count++; fault.signal();
    TEST_ASSERT(fault_fp->is_faulted());

    // Resetting the fault works.
    fault.unsignal();
    TEST_ASSERT_FALSE(fault_fp->is_faulted());
}

void test_fault_overridden_behavior() {
    StateFieldRegistryMock r;
    unsigned int control_cycle_count = 0;
    Fault fault("fault", 1, control_cycle_count);
    fault.add_to_registry(r);

    Fault* fault_fp = static_cast<Fault*>(r.find_writable_field_t<bool>("fault"));
    WritableStateField<bool>* override_fp = r.find_writable_field_t<bool>("fault.override");
    WritableStateField<bool>* suppress_fp = r.find_writable_field_t<bool>("fault.suppress");

    // The fault is signaled if overrided.
    override_fp->set(true);
    TEST_ASSERT(fault_fp->is_faulted());
    // Override is immune to the fault being unsignaled
    fault.unsignal();
    TEST_ASSERT(fault_fp->is_faulted());

    override_fp->set(false);

    // The fault should not be signaled if suppressed.
    // We'll cause the fault to be signaled first, and then suppress it.
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    TEST_ASSERT(fault_fp->is_faulted()); // Fault signaled
    suppress_fp->set(true);
    TEST_ASSERT_FALSE(fault_fp->is_faulted());
    // Fault should remain suppressed even when additional fault conditions are signaled
    control_cycle_count++; fault.signal();
    TEST_ASSERT_FALSE(fault_fp->is_faulted());

    // Fault should be signaled if suppressed and overrided (i.e. overrides take precedence.)
    fault.unsignal();
    override_fp->set(true);
    suppress_fp->set(true);
    TEST_ASSERT(fault_fp->is_faulted());
}

void test_process_commands(){
    StateFieldRegistryMock r;
    unsigned int control_cycle_count = 0;
    Fault fault("fault", 5, control_cycle_count);
    fault.add_to_registry(r);

    Fault* fault_fp = static_cast<Fault*>(r.find_writable_field_t<bool>("fault"));
    WritableStateField<bool>* override_fp = r.find_writable_field_t<bool>("fault.override");
    WritableStateField<bool>* suppress_fp = r.find_writable_field_t<bool>("fault.suppress");
    WritableStateField<bool>* unsignal_fp = r.find_writable_field_t<bool>("fault.unsignal");

    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    TEST_ASSERT_EQUAL(2, fault_fp->get_num_consecutive_signals());
    TEST_ASSERT_FALSE(fault_fp->is_faulted());

    // commanding an override modifies return of is_faulted in the current cycle
    // processing the override resets num_consecutive_signals to 0,
    control_cycle_count++; fault.signal(); override_fp->set(true);
    TEST_ASSERT_TRUE(fault_fp->is_faulted());
    TEST_ASSERT_EQUAL(0, fault_fp->get_num_consecutive_signals());

    // since fault was never formally faulted, releasing override returns fault to false
    control_cycle_count++; fault.signal(); override_fp->set(false);
    TEST_ASSERT_FALSE(fault_fp->is_faulted());

    // increment to 3 consecutive signals
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    TEST_ASSERT_EQUAL(3, fault_fp->get_num_consecutive_signals());

    // suppress a fault to prevent fault behavior
    // processing the suppress resets num_consecutive_signals to 0
    control_cycle_count++; fault.signal(); suppress_fp->set(true);
    TEST_ASSERT_FALSE(fault_fp->is_faulted());
    TEST_ASSERT_EQUAL(0, fault_fp->get_num_consecutive_signals());

    // fault tries to trip but doesn't change behavior due to supression
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    TEST_ASSERT_FALSE(fault_fp->is_faulted());
    TEST_ASSERT_EQUAL(6, fault_fp->get_num_consecutive_signals());

    // release suppression; since num_consecutive_signals was incrementing during suppression,
    // fault automatically triggers on supression release
    control_cycle_count++; fault.signal(); suppress_fp->set(false);
    TEST_ASSERT_TRUE(fault_fp->is_faulted());
    TEST_ASSERT_EQUAL(7, fault_fp->get_num_consecutive_signals());

    // Return to normal operation, signaling condition is not met
    control_cycle_count++; fault.unsignal();
    TEST_ASSERT_FALSE(fault_fp->is_faulted());
    TEST_ASSERT_EQUAL(0, fault_fp->get_num_consecutive_signals());

    // build up 6 signals, cause a fault trigger
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    TEST_ASSERT_TRUE(fault_fp->is_faulted());
    TEST_ASSERT_EQUAL(6, fault_fp->get_num_consecutive_signals());

    // send an unsignal command, even though we are in the signal condition
    control_cycle_count++; fault.signal(); unsignal_fp->set(true);
    TEST_ASSERT_FALSE(fault_fp->is_faulted());
    TEST_ASSERT_EQUAL(0, fault_fp->get_num_consecutive_signals());

    // begin accumulating signals as normal
    control_cycle_count++; fault.signal();
    TEST_ASSERT_FALSE(fault_fp->is_faulted());
    TEST_ASSERT_EQUAL(1, fault_fp->get_num_consecutive_signals());
}

void test_dynamic_persistence(){
    StateFieldRegistryMock r;
    unsigned int control_cycle_count = 0;
    Fault fault("fault", 5, control_cycle_count);
    fault.add_to_registry(r);

    Fault* fault_fp = static_cast<Fault*>(r.find_writable_field_t<bool>("fault"));
    WritableStateField<unsigned int>* persistence_fp = r.find_writable_field_t<unsigned int>("fault.persistence");

    // normal triggering of a fault
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    TEST_ASSERT_TRUE(fault_fp->is_faulted());
    TEST_ASSERT_EQUAL(6, fault_fp->get_num_consecutive_signals());

    // return to nominal operation of satellite
    control_cycle_count++; fault.unsignal();
    TEST_ASSERT_FALSE(fault_fp->is_faulted());
    TEST_ASSERT_EQUAL(0, fault_fp->get_num_consecutive_signals());

    // nominal satellite operation, and a command to change persistence to 1
    control_cycle_count++; fault.unsignal(); persistence_fp->set(1);
    TEST_ASSERT_FALSE(fault_fp->is_faulted());
    TEST_ASSERT_EQUAL(0, fault_fp->get_num_consecutive_signals());

    // fault now triggers after 2 signals()
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    TEST_ASSERT_TRUE(fault_fp->is_faulted());
    TEST_ASSERT_EQUAL(2, fault_fp->get_num_consecutive_signals());

    // command an increase in persistence
    // since signals <= persistence, fault is released
    control_cycle_count++; fault.signal(); 
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal(); persistence_fp->set(10);
    TEST_ASSERT_FALSE(fault_fp->is_faulted());
    TEST_ASSERT_EQUAL(6, fault_fp->get_num_consecutive_signals());

    // edit persistence to lower it below current num_consecutive_signals
    // instantly triggers due to previous accumulation
    control_cycle_count++; fault.signal(); persistence_fp->set(5);
    TEST_ASSERT_TRUE(fault_fp->is_faulted());
    TEST_ASSERT_EQUAL(7, fault_fp->get_num_consecutive_signals());
}

void test_control_task(){
    RUN_TEST(test_fault_normal_behavior);
    RUN_TEST(test_fault_overridden_behavior);
    RUN_TEST(test_process_commands);
    RUN_TEST(test_dynamic_persistence);
}

#ifdef DESKTOP
int main() {
    UNITY_BEGIN();
    test_control_task();
    return UNITY_END();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    UNITY_BEGIN();
    test_control_task();
    UNITY_END();
}

void loop() {}
#endif
