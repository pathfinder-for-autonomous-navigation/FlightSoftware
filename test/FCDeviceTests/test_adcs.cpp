#include <i2c_t3.h>
#include <unity.h>
#include <unity_test/unity_fixture.h>

bool adcs_setup;

TEST_GROUP(ADCSTests);

TEST_SETUP(ADCSTests) {
    if (!adcs_setup) {
        Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
        adcs_setup = true;
    }
    // ADCS Setup TODO
}

TEST_TEAR_DOWN(ADCSTests) {}

TEST_GROUP_RUNNER(ADCSTests) {}