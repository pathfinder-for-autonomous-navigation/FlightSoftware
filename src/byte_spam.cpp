#include <Arduino.h>
#include <HardwareSerial.h>

HardwareSerial fake_serial(Serial4);

#ifndef UNIT_TEST
void setup() {
    Serial.begin(9600);
    fake_serial.begin(115200);
    delay(2000);
    Serial.println("spam begin");
}

void loop() { 
    fake_serial.printf("here are some bytes");
    delay(50);
}
#endif