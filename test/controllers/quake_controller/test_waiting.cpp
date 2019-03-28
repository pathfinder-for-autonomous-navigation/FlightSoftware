#include <unity_fixture.h>
#include <ChRt.h>
#include "../../src/FCCode/controllers/controllers.hpp"
#include "../../src/FCCode/state/state_holder.hpp"
#include "../../src/FCCode/deployment_timer.hpp"

using namespace State::Quake;

TEST_GROUP(QuakeWaitingTests);
TEST_SETUP(QuakeWaitingTests) {
    chThdSleepMilliseconds(500);
    chThdDequeueAllI(&deployment_timer_waiting, (msg_t) 0);
    State::write(quake_state, QuakeState::WAITING, quake_state_lock);
}
TEST_TEAR_DOWN(QuakeWaitingTests) {
    
}

TEST(QuakeWaitingTests, test_1) {

}

TEST_GROUP_RUNNER(QuakeWaitingTests) {

}