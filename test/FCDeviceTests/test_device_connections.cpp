#include <HardwareSerial.h>
#include <i2c_t3.h>
#include <Gomspace/Gomspace.hpp>
#include <Piksi/Piksi.hpp>
#include <QLocate/QLocate.hpp>
#include <unity.h>
#include <unity_test/unity_fixture.h>

using namespace Devices;

TEST_GROUP(DeviceDisconnectedTests);

TEST_SETUP(DeviceDisconnectedTests) {}
TEST_TEAR_DOWN(DeviceDisconnectedTests) {}

TEST(DeviceDisconnectedTests, test_gomspace_disconnected) {
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM); // Gomspace
    Gomspace::eps_hk_t gomspace_data;
    Gomspace gs(&gomspace_data, Wire, Gomspace::ADDRESS);
    gs.setup();
    TEST_ASSERT_FALSE(gs.is_functional());
}

TEST(DeviceDisconnectedTests, test_piksi_disconnected) {
    Serial4.begin(9600); // Piksi
    Piksi piksi(Serial4);
    piksi.setup();
    TEST_ASSERT_FALSE(piksi.is_functional());
}

TEST(DeviceDisconnectedTests, test_quake_disconnected) {
    Serial3.begin(9600); // Quake
    QLocate quake(&Serial3, 20);
    quake.setup();
    TEST_ASSERT_FALSE(quake.is_functional());
}

TEST(DeviceDisconnectedTests, test_pressure_sensors_disconnected) {
    // TODO
    TEST_ASSERT_EQUAL(2,2);
}

TEST(DeviceDisconnectedTests, test_adcs_disconnected)  {
    // TODO
    TEST_ASSERT_EQUAL(2,2);
}

TEST_GROUP_RUNNER(DeviceDisconnectedTests) {
    RUN_TEST_CASE(DeviceDisconnectedTests, test_gomspace_disconnected);
    RUN_TEST_CASE(DeviceDisconnectedTests, test_piksi_disconnected);
    RUN_TEST_CASE(DeviceDisconnectedTests, test_quake_disconnected);
    RUN_TEST_CASE(DeviceDisconnectedTests, test_pressure_sensors_disconnected);
    RUN_TEST_CASE(DeviceDisconnectedTests, test_adcs_disconnected);
}