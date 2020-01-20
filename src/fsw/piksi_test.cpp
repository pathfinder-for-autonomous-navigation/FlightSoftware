#include <Arduino.h>
#include <Piksi.hpp>
#include <HardwareSerial.h>

Devices::Piksi piksi("piksi", Serial2);

#ifndef UNIT_TEST
void setup() {
    piksi.setup();
    Serial.begin(9600);
    delay(2000);
    Serial.println(piksi.is_functional());
}

void loop() { }
#endif