#include <unity_fixture.h>
#include <ChRt.h>
#include "../../src/FCCode/controllers/controllers.hpp"
#include "../../src/FCCode/state/state_holder.hpp"
#include "../../src/FCCode/deployment_timer.hpp"

TEST_GROUP(ADCSInitializationTests);
TEST_SETUP(ADCSInitializationTests) {}
TEST_TEAR_DOWN(ADCSInitializationTests) {}

TEST(ADCSInitializationTests, test_exits_deployment_timer_automatically) {
    chThdSleepSeconds(DEPLOYMENT_LENGTH);
    TEST_ASSERT(chThdQueueIsEmptyI(&deployment_timer_waiting));
}

TEST(ADCSInitializationTests, test_exits_deployment_with_trigger_successfully) {
    chThdDequeueAllI(&deployment_timer_waiting, (msg_t) 0);
    TEST_ASSERT(chThdQueueIsEmptyI(&deployment_timer_waiting));
}

TEST_GROUP_RUNNER(ADCSInitializationTests) {
    RUN_TEST_CASE(ADCSInitializationTests, test_exits_deployment_timer_automatically);
    RUN_TEST_CASE(ADCSInitializationTests, test_exits_deployment_with_trigger_successfully);
}