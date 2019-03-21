#include <unity_fixture.h>

TEST_GROUP(ADCSDetumbleTests);
TEST_SETUP(ADCSDetumbleTests) {}
TEST_TEAR_DOWN(ADCSDetumbleTests) {}

TEST(ADCSDetumbleTests, test_stays_in_detumble_if_angular_rate_not_ok) {
    TEST_ASSERT(false);
}

TEST(ADCSDetumbleTests, test_exits_detumble_if_angular_rate_ok) {
    TEST_ASSERT(false);
}

TEST(ADCSDetumbleTests, test_exits_detumble_if_detumble_times_out) {
    TEST_ASSERT(false);
}

TEST(ADCSDetumbleTests, test_exiting_detumble_on_angular_rate_dequeues_waiting_threads) {
    TEST_ASSERT(false);
}

TEST(ADCSDetumbleTests, test_exiting_detumble_on_timeout_dequeues_waiting_threads) {
    TEST_ASSERT(false);
}

TEST_GROUP_RUNNER(ADCSDetumbleTests) {
    RUN_TEST_CASE(ADCSDetumbleTests, test_stays_in_detumble_if_angular_rate_not_ok);
    RUN_TEST_CASE(ADCSDetumbleTests, test_exits_detumble_if_angular_rate_ok);
    RUN_TEST_CASE(ADCSDetumbleTests, test_exits_detumble_if_detumble_times_out);
    RUN_TEST_CASE(ADCSDetumbleTests, test_exiting_detumble_on_angular_rate_dequeues_waiting_threads);
    RUN_TEST_CASE(ADCSDetumbleTests, test_exiting_detumble_on_timeout_dequeues_waiting_threads);
}