#include <unity_fixture.h>
#include <ChRt.h>
#include "../../src/FCCode/controllers/controllers.hpp"
#include "../../src/FCCode/state/state_holder.hpp"
#include "../../src/FCCode/deployment_timer.hpp"

TEST_GROUP(ADCSSafeHoldTests);
TEST_SETUP(ADCSSafeHoldTests) {}
TEST_TEAR_DOWN(ADCSSafeHoldTests) {}

TEST(ADCSSafeHoldTests, test_magnetorquers_off) {

}

TEST(ADCSSafeHoldTests, test_wheels_off) {
    
}

TEST(ADCSSafeHoldTests, cmding_attitude_does_not_change_actuation) {
    
}

TEST(ADCSSafeHoldTests, cmding_attitude_does_not_change_actuation) {
    
}

TEST_GROUP_RUNNER(ADCSSafeHoldTests) {
    RUN_TEST_CASE(ADCSSafeHoldTests, test_magnetorquers_off);
}