#include <Arduino.h>

void setup() {
}

void loop() {
    unsigned char pin = 3; // Valve 1
    unsigned char pin2 = 4; // Valve 1
    digitalWrite(pin, HIGH);
    delay(2);
    digitalWrite(pin2, HIGH);
    delay(50);
    digitalWrite(pin, LOW);
    digitalWrite(pin2, LOW);
}