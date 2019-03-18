#include <unity_test/unity_fixture.h>

TEST_GROUP(UtilsTests);
TEST_SETUP(UtilsTests) {}
TEST_TEAR_DOWN(UtilsTests) {}

TEST_GROUP_RUNNER(UtilsTests) {
    RUN_TEST_GROUP(CircularBufferTests);
    RUN_TEST_GROUP(CircularStackTests);
    RUN_TEST_GROUP(CommsUtilsTests);
    RUN_TEST_GROUP(RWMutexTests);
}