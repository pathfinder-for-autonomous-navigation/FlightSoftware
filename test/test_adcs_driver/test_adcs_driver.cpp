#include <unity.h>
//#include <Arduino.h>
//#include <i2c_t3.h>
#include <ADCS.hpp>
#include <array>
#include <adcs_constants.hpp>

Devices::ADCS adcs(Wire, Devices::ADCS::ADDRESS);

void setup() {
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
    adcs.setup();
}

void test_havt() {
    TEST_ASSERT_TRUE(false);
}

int main(void) {
    delay(5000);
    Serial.begin(9600);
    //pinMode(13, OUTPUT);
    while (!Serial)
        ;
    adcs.setup();
    UNITY_BEGIN();
    RUN_TEST(test_havt);
    UNITY_END();
    return 0;
}
