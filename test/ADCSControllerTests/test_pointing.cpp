#include <unity_fixture.h>

TEST_GROUP(ADCSPointingTests);
TEST_SETUP(ADCSPointingTests) {}
TEST_TEAR_DOWN(ADCSPointingTests) {}

TEST(ADCSPointingTests, test_command_conversion_to_eci_does_nothing_if_command_is_already_in_eci) {
    TEST_ASSERT(false);
}

TEST(ADCSPointingTests, test_command_conversion_to_eci_correctly_converts_command_to_eci_if_command_in_lvlh) {
    TEST_ASSERT(false);
}

TEST_GROUP_RUNNER(ADCSPointingTests) {
    RUN_TEST_CASE(ADCSPointingTests, test_command_conversion_to_eci_does_nothing_if_command_is_already_in_eci);
    RUN_TEST_CASE(ADCSPointingTests, test_command_conversion_to_eci_correctly_converts_command_to_eci_if_command_in_lvlh);
}