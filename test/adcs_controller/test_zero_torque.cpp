#include <unity_fixture.h>
#include "../../src/FCCode/state/state_holder.hpp"
#include "../../src/FCCode/controllers/adcs/adcs_helpers.hpp"
#include "../../src/FCCode/deployment_timer.hpp"

using namespace State::ADCS;

TEST_GROUP(ADCSZeroTorqueTests);
TEST_SETUP(ADCSZeroTorqueTests) {
    chThdSleepMilliseconds(500);
    chThdDequeueAllI(&deployment_timer_waiting, (msg_t) 0);
    State::write(adcs_state, ADCSState::ZERO_TORQUE, adcs_state_lock);
}
TEST_TEAR_DOWN(ADCSZeroTorqueTests) {
    TEST_ASSERT(chThdQueueIsEmptyI(&deployment_timer_waiting));
}

TEST(ADCSZeroTorqueTests, test_zero_torque_commands_same_wheel_speed) {
    std::array<float, 3> speed_cmds = State::read(rwa_speed_cmds, adcs_state_lock);
    for(int i = 0; i < 10; i++) {
        bool equal = speed_cmds == State::read(rwa_speed_cmds, adcs_state_lock);
        TEST_ASSERT(equal);
        chThdSleepMilliseconds(RTOSTasks::LoopTimes::ADCS);
    }
}

TEST(ADCSZeroTorqueTests, test_zero_torque_commands_zero_mtr) {
    for(int i = 0; i < 3; i++) TEST_ASSERT_EQUAL(State::read(mtr_cmds[i], adcs_state_lock), 0);
}

TEST_GROUP_RUNNER(ADCSZeroTorqueTests) {
    RUN_TEST_CASE(ADCSZeroTorqueTests, test_zero_torque_commands_same_wheel_speed);
    RUN_TEST_CASE(ADCSZeroTorqueTests, test_zero_torque_commands_zero_mtr);
}