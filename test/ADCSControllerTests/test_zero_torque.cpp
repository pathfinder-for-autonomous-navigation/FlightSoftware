#include <unity_fixture.h>

TEST_GROUP(ADCSZeroTorqueTests);
TEST_SETUP(ADCSZeroTorqueTests) {}
TEST_TEAR_DOWN(ADCSZeroTorqueTests) {}

TEST(ADCSZeroTorqueTests, test_zero_torque_commands_same_wheel_speed) {
    TEST_ASSERT(false);
}

TEST(ADCSZeroTorqueTests, test_zero_torque_commands_zero_mtr) {
    TEST_ASSERT(false);
}

TEST_GROUP_RUNNER(ADCSZeroTorqueTests) {
    RUN_TEST_CASE(ADCSZeroTorqueTests, test_zero_torque_commands_same_wheel_speed);
    RUN_TEST_CASE(ADCSZeroTorqueTests, test_zero_torque_commands_zero_mtr);
}