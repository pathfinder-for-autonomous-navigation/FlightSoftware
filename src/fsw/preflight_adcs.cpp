// REMOVE BEFORE FLIGHT "RESET" CODE
// Resets satellite to original configuration prior to flight.

#include <Arduino.h>

#ifndef UNIT_TEST
void setup() {
    Serial.begin(9600);
    delay(1000);
    Serial.println("Beginning PAN ADCS reset.");
    // Set all ramps to zero, etc.
    Serial.println("ADCS is reset!");
}

void loop() {
    // Do nothing.
}
#endif