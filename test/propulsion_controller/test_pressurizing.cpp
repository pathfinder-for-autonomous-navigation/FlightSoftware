#include <unity_fixture.h>
#include <ChRt.h>
#include "../../src/FCCode/controllers/controllers.hpp"
#include "../../src/FCCode/state/state_holder.hpp"
#include "../../src/FCCode/deployment_timer.hpp"

using namespace State::Propulsion;

TEST_GROUP(PropulsionPressurizingTests);
TEST_SETUP(PropulsionPressurizingTests) {
    chThdSleepMilliseconds(500);
    chThdDequeueAllI(&deployment_timer_waiting, (msg_t) 0);
    State::write(propulsion_state, PropulsionState::PRESSURIZING, propulsion_state_lock);
}
TEST_TEAR_DOWN(PropulsionPressurizingTests) {
    
}

TEST(PropulsionPressurizingTests, test_1) {

}

TEST_GROUP_RUNNER(PropulsionPressurizingTests) {
    
}