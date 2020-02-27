#include <common/StateFieldRegistry.hpp>
#include <common/Fault.hpp>

#include <unity.h>

void test_foo() {
    StateFieldRegistry registry;
}

void test_faults() {
    StateFieldRegistry registry;

    // Add fault to registry
    unsigned int control_cycle_count=1;
    Fault f("fault", 1, control_cycle_count);
    TEST_ASSERT_TRUE(registry.add_fault(&f));

    // Check that the fault and its writable fields were added to the registry
    TEST_ASSERT_NOT_NULL(registry.find_fault("fault"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("fault"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("fault.suppress"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("fault.override"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("fault.unsignal"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("fault.persistence"));

    // We shouldn't be able to add a fault that already exists
    TEST_ASSERT_FALSE(registry.add_fault(&f));

    // We shouldn't be able to find a fault that doesn't exist
    TEST_ASSERT_FALSE(registry.find_fault("fake_fault"));
}

void test_state_field_registry() {
    UNITY_BEGIN();
    RUN_TEST(test_foo);
    RUN_TEST(test_faults);
    UNITY_END();
}

#ifdef DESKTOP
int main(int argc, char *argv[]) {
    test_state_field_registry();
    return 0;
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_state_field_registry();
}

void loop() {}
#endif
