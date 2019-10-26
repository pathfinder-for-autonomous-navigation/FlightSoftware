#include "../StateFieldRegistryMock.hpp"

#include <unity.h>
#include <iostream>

void test_valid_initialization() {
    StateFieldRegistryMock registry; 
}

/**
 * @brief Test create functions that don't require any arguments
 * besides the name of the field.
 */
void test_create_readable_field_noargs() {
    StateFieldRegistryMock registry;

    auto foo_ptr = registry.create_readable_field<bool>("foo");
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo"));

    auto foo_ptr2 = registry.create_readable_field<gps_time_t>("foo2");
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo2"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo2"));

    auto foo_ptr3 = registry.create_readable_field<f_quat_t>("foo3");
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo3"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo3"));

    auto foo_ptr4 = registry.create_readable_field<d_quat_t>("foo4");
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo4"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo4"));
}

void test_create_writable_field_noargs() {
    StateFieldRegistryMock registry;

    auto foo_ptr = registry.create_writable_field<bool>("foo");
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo"));

    auto foo_ptr2 = registry.create_writable_field<gps_time_t>("foo2");
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo2"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo2"));

    auto foo_ptr3 = registry.create_writable_field<f_quat_t>("foo3");
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo3"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo3"));

    auto foo_ptr4 = registry.create_readable_field<d_quat_t>("foo4");
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo4"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo4"));
}

void test_create_readable_field_args() {
    StateFieldRegistryMock registry;

    auto foo_ptr = registry.create_readable_field<signed int>("foo", -1, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo"));

    auto foo_ptr2 = registry.create_readable_field<unsigned int>("foo2", 1, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo2"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo2"));

    auto foo_ptr3 = registry.create_readable_field<float>("foo3", 2.2, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo3"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo3"));

    auto foo_ptr4 = registry.create_readable_field<double>("foo4", 2.2, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo4"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo4"));
}

void test_create_writable_field_args() {
    StateFieldRegistryMock registry;

    auto foo_ptr = registry.create_writable_field<signed int>("foo", -1, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo"));

    auto foo_ptr2 = registry.create_writable_field<unsigned int>("foo2", 1, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo2"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo2"));

    auto foo_ptr3 = registry.create_writable_field<float>("foo3", 2.2, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo3"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo3"));

    auto foo_ptr4 = registry.create_writable_field<double>("foo4", 2.2, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo4"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo4"));
}

void test_create_readable_vector_field_args() {
    StateFieldRegistryMock registry;

    auto foo_ptr = registry.create_readable_vector_field<float>("foo", 0, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo"));

    auto foo_ptr2 = registry.create_readable_vector_field<double>("foo2", 0, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo2"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo2"));
}

void test_create_writable_vector_field_args() {
    StateFieldRegistryMock registry;

    auto foo_ptr = registry.create_writable_vector_field<float>("foo", 0, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo"));

    auto foo_ptr2 = registry.create_writable_vector_field<double>("foo2", 0, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo2"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo2"));
}

int test_state_field_registry_mock() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    RUN_TEST(test_create_readable_field_noargs);
    RUN_TEST(test_create_writable_field_noargs);
    RUN_TEST(test_create_readable_field_args);
    RUN_TEST(test_create_writable_field_args);
    RUN_TEST(test_create_readable_vector_field_args);
    RUN_TEST(test_create_writable_vector_field_args);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_state_field_registry_mock();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_state_field_registry_mock();
}

void loop() {}
#endif
