#include <common/StateFieldRegistry.hpp>

#include <unity.h>

void test_internal_fields() {
    StateFieldRegistry registry;
    InternalStateField<bool> isf("field");

    // We should be able to add a field successfully to the
    // registry.
    TEST_ASSERT_TRUE(registry.add_internal_field(&isf));
    // We should not be able to add a field that already
    // exists in the registry.
    TEST_ASSERT_FALSE(registry.add_internal_field(&isf));
    TEST_ASSERT_NULL(registry.find_internal_field("nonexistent_field"));
    TEST_ASSERT_EQUAL(&isf, registry.find_internal_field("field"));
}

void test_readable_fields() {
    StateFieldRegistry registry;
    ReadableStateField<bool> rsf("field");

    // We should be able to add a field successfully to the
    // registry.
    TEST_ASSERT_TRUE(registry.add_readable_field(&rsf));
    // We should not be able to add a field that already
    // exists in the registry.
    TEST_ASSERT_FALSE(registry.add_readable_field(&rsf));
    TEST_ASSERT_NULL(registry.find_readable_field("nonexistent_field"));
    TEST_ASSERT_EQUAL(&rsf, registry.find_readable_field("field"));
}

void test_writable_fields() {
    StateFieldRegistry registry;
    WritableStateField<bool> wsf("field");

    // We should be able to add a field successfully to the
    // registry.
    TEST_ASSERT_TRUE(registry.add_writable_field(&wsf));
    // We should not be able to add a field that already
    // exists in the registry.
    TEST_ASSERT_FALSE(registry.add_writable_field(&wsf));
    TEST_ASSERT_NULL(registry.find_writable_field("nonexistent_field"));
    TEST_ASSERT_EQUAL(&wsf, registry.find_writable_field("field"));
    // We should also be able to access writable fields as
    // readable fields.
    TEST_ASSERT_EQUAL(&wsf, registry.find_readable_field("field"));
}

void test_faults() {
    StateFieldRegistry registry;
    unsigned int ccno = 0;
    Fault fault("fault", 1, ccno);

    // We should be able to add a fault successfully to the
    // registry.
    TEST_ASSERT_TRUE(registry.add_fault(&fault));
    // We should not be able to add a fault that already
    // exists in the registry.
    TEST_ASSERT_FALSE(registry.add_fault(&fault));
    TEST_ASSERT_NULL(registry.find_fault("nonexistent_fault"));
    TEST_ASSERT_EQUAL(&fault, registry.find_fault("fault"));
}

void test_state_field_registry() {
    UNITY_BEGIN();
    RUN_TEST(test_internal_fields);
    RUN_TEST(test_readable_fields);
    RUN_TEST(test_writable_fields);
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
