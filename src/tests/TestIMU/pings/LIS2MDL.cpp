#include <Arduino.h>
#include <HardwareSerial.h>
#include <LIS2MDL/LIS2MDL.h>
#include <i2c_t3.h>
using namespace Devices;

LIS2MDL imu(Wire);

void setup() {
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_ISR);
    Serial.begin(9600);
    imu.setup();
}

void loop() {
    imu.single_comp_test();
}