#include <unity_fixture.h>
#include <ChRt.h>
#include "../../src/FCCode/controllers/controllers.hpp"
#include "../../src/FCCode/state/state_holder.hpp"
#include "../../src/FCCode/deployment_timer.hpp"

using namespace State::Propulsion;

TEST_GROUP(PropulsionFiringTests);
TEST_SETUP(PropulsionFiringTests) {
    chThdSleepMilliseconds(500);
    chThdDequeueAllI(&deployment_timer_waiting, (msg_t) 0);
    State::write(propulsion_state, PropulsionState::FIRING, propulsion_state_lock);
}
TEST_TEAR_DOWN(PropulsionFiringTests) {
    
}

TEST(PropulsionFiringTests, test_1) {

}

TEST_GROUP_RUNNER(PropulsionFiringTests) {
    
}