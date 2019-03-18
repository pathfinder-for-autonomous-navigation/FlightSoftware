#include <unity_test/unity_fixture.h>
#include <Arduino.h>

TEST_GROUP(CircularBufferTests);
TEST_SETUP(CircularBufferTests) {}
TEST_TEAR_DOWN(CircularBufferTests) {}

TEST(CircularBufferTests, test_empty) {
    
}

TEST_GROUP_RUNNER(CircularBufferTests) {
    RUN_TEST_CASE(CircularBufferTests, test_empty);
}