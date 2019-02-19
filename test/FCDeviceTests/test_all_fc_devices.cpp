#include <unity_test/unity_fixture.h>

TEST_GROUP(FCDeviceTests);
TEST_SETUP(FCDeviceTests) {}
TEST_TEAR_DOWN(FCDeviceTests) {}

TEST_GROUP_RUNNER(FCDeviceTests) {
    RUN_TEST_GROUP(DeviceDisconnectedTests);
    RUN_TEST_GROUP(GomspaceTests);
    RUN_TEST_GROUP(PiksiTests);
    RUN_TEST_GROUP(QuakeTests);
    RUN_TEST_GROUP(DockingMotorTests);
    RUN_TEST_GROUP(ADCSTests);
}