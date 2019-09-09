#include "../../lib/utils/Nameable.hpp"
#include "../../src/FCCode/StateField.hpp"

#include <unity.h>

void test_internal_state_field() {
    // Test setter and getter for state field.

    InternalStateField<float> isf("field");
    TEST_ASSERT_FALSE(isf.is_readable());
    TEST_ASSERT_FALSE(isf.is_writable());

    isf.set(2.0f);
    TEST_ASSERT_EQUAL(isf.get(), 2);
}

void test_readable_state_field() {
    Serializer<float> serializer(0, 10, 10);
    ReadableStateField<float> rsf("field", serializer);
    TEST_ASSERT(rsf.is_readable());
    TEST_ASSERT_FALSE(rsf.is_writable());

    rsf.set(2.0f);
    TEST_ASSERT_EQUAL(rsf.get(), 2);
}

void test_state_field() { RUN_TEST(test_internal_state_field); }

int main(int argc, char *argv[]) {
    UNITY_BEGIN();
    test_state_field();
    UNITY_END();
    return 0;
}
