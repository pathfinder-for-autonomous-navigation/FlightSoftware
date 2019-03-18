#include <unity_test/unity_fixture.h>
#include <circular_stack.hpp>
#include <array>
#include <Arduino.h>

TEST_GROUP(CommsUtilsTests);
TEST_SETUP(CommsUtilsTests) {

}
TEST_TEAR_DOWN(CommsUtilsTests) {}

// Tests if buffer works fine with just one element.
TEST(CommsUtilsTests, test_one_element) {
}

TEST_GROUP_RUNNER(CommsUtilsTests) {
    RUN_TEST_CASE(CommsUtilsTests, test_one_element);
}