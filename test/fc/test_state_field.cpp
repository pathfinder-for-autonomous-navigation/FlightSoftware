#include "../../src/FCCode/StateField.hpp"

#include <unity.h>

void test_something() { TEST_ASSERT(true); }

void test_state_field() { RUN_TEST(test_something); }

int main(int argc, char *argv[]) {
    UNITY_BEGIN();
    test_state_field();
    UNITY_END();
    return 0;
}
