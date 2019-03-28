#include <unity_fixture.h>
#include <ChRt.h>
#include "../../src/FCCode/controllers/controllers.hpp"
#include "../../src/FCCode/state/state_holder.hpp"
#include "../../src/FCCode/deployment_timer.hpp"

using namespace State::ADCS;

TEST_GROUP(ADCSDetumbleTests);
TEST_SETUP(ADCSDetumbleTests) {
    chThdSleepMilliseconds(500);
    chThdDequeueAllI(&deployment_timer_waiting, (msg_t) 0);
    State::write(adcs_state, ADCSState::ADCS_DETUMBLE, adcs_state_lock);
}
TEST_TEAR_DOWN(ADCSDetumbleTests) {
    TEST_ASSERT(chThdQueueIsEmptyI(&deployment_timer_waiting));
}

TEST(ADCSDetumbleTests, test_stays_in_detumble_if_angular_rate_not_ok) {
    State::ADCS::cur_ang_rate.fill(10);
    chThdSleepSeconds(4);
    TEST_ASSERT(State::read(adcs_state, adcs_state_lock) == ADCSState::ADCS_DETUMBLE);
}

TEST(ADCSDetumbleTests, test_exits_detumble_if_angular_rate_ok) {
    TEST_ASSERT(State::read(adcs_state, adcs_state_lock) == ADCSState::ADCS_DETUMBLE);
    State::ADCS::cur_ang_rate.fill(0);
    chThdSleepMilliseconds(2 * RTOSTasks::LoopTimes::ADCS);
    TEST_ASSERT(State::read(adcs_state, adcs_state_lock) == ADCSState::ZERO_TORQUE);
}

TEST(ADCSDetumbleTests, test_exits_detumble_if_detumble_times_out) {
    TEST_ASSERT(State::read(adcs_state, adcs_state_lock) == ADCSState::ADCS_DETUMBLE);
    chThdSleepSeconds(5);
    TEST_ASSERT(State::read(adcs_state, adcs_state_lock) == ADCSState::ZERO_TORQUE);
}

TEST_GROUP_RUNNER(ADCSDetumbleTests) {
    RUN_TEST_CASE(ADCSDetumbleTests, test_stays_in_detumble_if_angular_rate_not_ok);
    RUN_TEST_CASE(ADCSDetumbleTests, test_exits_detumble_if_angular_rate_ok);
    RUN_TEST_CASE(ADCSDetumbleTests, test_exits_detumble_if_detumble_times_out);
}