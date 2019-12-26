#include "../../src/FCCode/Fault.hpp"
#include "../StateFieldRegistryMock.hpp"
#include <unity.h>

/**
 * @brief Test normal behavior of fault without overrides or supressings.
 */
void test_fault_normal_behavior() {
    unsigned int control_cycle_count = 0;

    // Test constructors
    Fault fault("fault", 5, control_cycle_count);
    TEST_ASSERT_FALSE(fault.is_faulted());
    Fault fault2("fault", 5, control_cycle_count, true);
    TEST_ASSERT(fault2.is_faulted());

    // Test adding to registry
    StateFieldRegistryMock r;
    TEST_ASSERT(fault.add_to_registry(r));
    TEST_ASSERT(r.find_readable_field_t<bool>("fault"));
    TEST_ASSERT(r.find_writable_field_t<bool>("fault.override"));
    TEST_ASSERT(r.find_writable_field_t<bool>("fault.suppress"));

    // Test that fault is not signaled when signal condition happens under the persistence
    for(int i = 0; i < 5; i++) {
        fault.signal();
        TEST_ASSERT_FALSE(fault.is_faulted());
        control_cycle_count++;
    }

    // Test that signaling the fault again has no effect if the control cycle count
    // is not also incremented.
    fault.signal();
    TEST_ASSERT_FALSE(fault.is_faulted());

    // Resetting the fault does not cause the fault to be signaled, but does cause
    // the num_conseuctive_faults to reset. Resultantly, incrementing the control
    // cycle count and signaling does not cause the fault to be signaled.
    fault.unsignal();
    control_cycle_count++;
    fault.signal();
    TEST_ASSERT_FALSE(fault.is_faulted());

    // Keep signaling the fault until we're just underneath the fault's persistence
    // threshold.
    for(int i = 0; i < 4; i++, control_cycle_count++) fault.signal();
    // Incrementing the control cycle count now causes the fault to be signaled.
    fault.signal();
    control_cycle_count++;
    TEST_ASSERT(fault.is_faulted());

    // Resetting the fault works.
    fault.unsignal();
    TEST_ASSERT_FALSE(fault.is_faulted());
}

void test_fault_overridden_behavior() {
    StateFieldRegistryMock r;
    unsigned int control_cycle_count = 0;
    Fault fault("fault", 1, control_cycle_count);
    fault.add_to_registry(r);
    WritableStateField<bool>* override_fp = r.find_writable_field_t<bool>("fault.override");
    WritableStateField<bool>* suppress_fp = r.find_writable_field_t<bool>("fault.suppress");

    // The fault is signaled if overrided.
    override_fp->set(true);
    TEST_ASSERT(fault.is_faulted());
    override_fp->set(false);

    // The fault is not signaled if suppressed.
    control_cycle_count++; fault.signal();
    control_cycle_count++; fault.signal();
    TEST_ASSERT(fault.is_faulted());
    suppress_fp->set(true);
    TEST_ASSERT_FALSE(fault.is_faulted());

    // The fault is signaled if suppressed and overrided (overrides take precedence.)
    fault.unsignal();
    override_fp->set(true);
    suppress_fp->set(true);
    TEST_ASSERT(fault.is_faulted());
}

#ifdef DESKTOP
int main() {
    UNITY_BEGIN();
    RUN_TEST(test_fault_normal_behavior);
    RUN_TEST(test_fault_overridden_behavior);
    return UNITY_END();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    UNITY_BEGIN();
    RUN_TEST(test_fault_behavior);
    UNITY_END();
}

void loop() {}
#endif
