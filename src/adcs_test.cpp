#include <Arduino.h>
#include <i2c_t3.h>
#include <ADCS.hpp>

Devices::ADCS adcs("adcs", Wire, Devices::ADCS::ADDRESS);

#ifndef UNIT_TEST
void setup() {
    adcs.setup();
}

void loop() {
    Serial.println(adcs.is_functional());
    delay(100);
}
#endif
