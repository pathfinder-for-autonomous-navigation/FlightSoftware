#include <Arduino.h>
#include <i2c_t3.h>
#include <ADS1015.hpp>

ADS1015 pot(Wire, ADS1015::ADS1015_ADDR::GND, 11);

void setup() {
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
}

void loop() {}