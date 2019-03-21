#include <unity_fixture.h>
#include <array>
#include "../../src/FCCode/state/state_holder.hpp"
#include "../../src/FCCode/controllers/adcs/adcs_helpers.hpp"
#include "../../src/FCCode/deployment_timer.hpp"

using namespace State::ADCS;

TEST_GROUP(ADCSPointingTests);

std::array<float, 4> attitude_eci = {0.3,0.3,0.3,0.8544};
TEST_SETUP(ADCSPointingTests) {
    State::write(cmd_attitude, attitude_eci, adcs_state_lock);
    chThdSleepMilliseconds(500);
    chThdDequeueAllI(&deployment_timer_waiting, (msg_t) 0);
    State::write(adcs_state, ADCSState::POINTING, adcs_state_lock);
}
TEST_TEAR_DOWN(ADCSPointingTests) {
    TEST_ASSERT(chThdQueueIsEmptyI(&deployment_timer_waiting));
}

TEST(ADCSPointingTests, test_command_conversion_to_eci_does_nothing_if_command_is_already_in_eci) {
    State::write(cmd_attitude_frame, PointingFrame::ECI, adcs_state_lock);
    std::array<float, 4> result;
    ADCSControllers::get_command_attitude_in_eci(&result);
    bool attitudes_equal = attitude_eci == result;
    TEST_ASSERT(attitudes_equal);
}

TEST(ADCSPointingTests, test_command_conversion_to_eci_converts_command_to_eci_if_command_in_lvlh) {
    State::write(cmd_attitude_frame, PointingFrame::LVLH, adcs_state_lock);
    std::array<float, 4> result;
    ADCSControllers::get_command_attitude_in_eci(&result);
    bool attitudes_equal = attitude_eci == result;
    TEST_ASSERT_FALSE(attitudes_equal);
}

TEST_GROUP_RUNNER(ADCSPointingTests) {
    RUN_TEST_CASE(ADCSPointingTests, test_command_conversion_to_eci_does_nothing_if_command_is_already_in_eci);
    RUN_TEST_CASE(ADCSPointingTests, test_command_conversion_to_eci_converts_command_to_eci_if_command_in_lvlh);
}