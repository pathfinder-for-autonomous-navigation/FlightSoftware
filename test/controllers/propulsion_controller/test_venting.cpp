#include <unity_fixture.h>
#include <ChRt.h>
#include "../../src/FCCode/controllers/controllers.hpp"
#include "../../src/FCCode/state/state_holder.hpp"
#include "../../src/FCCode/deployment_timer.hpp"

using namespace State::Propulsion;

TEST_GROUP(PropulsionVentingTests);
TEST_SETUP(PropulsionVentingTests) {
    chThdSleepMilliseconds(500);
    chThdDequeueAllI(&deployment_timer_waiting, (msg_t) 0);
    State::write(propulsion_state, PropulsionState::VENTING, propulsion_state_lock);
}
TEST_TEAR_DOWN(PropulsionVentingTests) {
    
}

TEST(PropulsionVentingTests, test_vent_inner_tank_temp_too_high) { }

TEST(PropulsionVentingTests, test_vent_outer_tank_temp_too_high) { }

TEST(PropulsionVentingTests, test_vent_outer_tank_pressure_too_high) { }

TEST_GROUP_RUNNER(PropulsionVentingTests) {
    RUN_TEST_CASE(PropulsionVentingTests, test_vent_inner_tank_temp_too_high);
    RUN_TEST_CASE(PropulsionVentingTests, test_vent_outer_tank_temp_too_high);
    RUN_TEST_CASE(PropulsionVentingTests, test_vent_outer_tank_pressure_too_high);
}