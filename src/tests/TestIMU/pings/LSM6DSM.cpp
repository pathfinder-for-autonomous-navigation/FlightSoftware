#include <Arduino.h>
#include <LSM6DSM/LSM6DSM.h>

using namespace Devices;

LSM6DSM imu(Wire, 0, 0, LSM6DSM::ADDRESS);

void setup() {
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_ISR);
    Serial.begin(9600);
    imu.setup();
}

void loop() {
    Serial.printf("Responding?: %d\n", imu.get_chip_id() == 0x6A);
    delay(1000);
}