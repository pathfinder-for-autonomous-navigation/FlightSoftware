#include "../../lib/utils/Nameable.hpp"
#include "../../src/FCCode/StateField.hpp"

#include <unity.h>

void test_internal_state_field() {
    InternalStateField<float> isf("field");
    TEST_ASSERT_FALSE(isf.is_readable());
    TEST_ASSERT_FALSE(isf.is_writable());

    isf.set(2);
    TEST_ASSERT_EQUAL(isf.get(), 2);
}

void test_state_field() { 
    RUN_TEST(test_internal_state_field);
}

int main(int argc, char *argv[]) {
    UNITY_BEGIN();
    test_state_field();
    UNITY_END();
    return 0;
}
