#include <Arduino.h>
#include <HardwareSerial.h>
#include <MMC5883MA/MMC5883MA.h>
#include <i2c_t3.h>
using namespace Devices;

MMC5883MA imu(Wire, MMC5883MA::ADDR);

void setup() {
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_ISR);
    Serial.begin(9600);
    imu.setup();
}

void loop() {
    Serial.println(imu.i2c_ping());
    delay(1000);
}