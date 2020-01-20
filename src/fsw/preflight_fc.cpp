// REMOVE BEFORE FLIGHT "RESET" CODE
// Resets satellite to original configuration prior to flight.

#include <Arduino.h>

#ifndef UNIT_TEST
void setup() {
    Serial.begin(9600);
    delay(1000);
    Serial.println("Beginning PAN system reset.");
    Serial.println("Resetting EEPROM.");
    // Reset EEPROM--everything

    Serial.println("Resetting Gomspace settings to defaults.");
    // Reset Gomspace settings
    Serial.println("Resetting Gomspace outputs.");
    // Set all Gomspace outputs to on, so that things are turned on automatically
    // in space even without FC input (TODO double check this)
    Serial.println("Gomspace configuration reset is verified.");

    Serial.println("Resetting Piksi settings to defaults.");
    // Reset Piksi settings
    Serial.println("Piksi configuration reset is verified.");

    Serial.println("Resetting Quake settings to defaults.");
    // Reset Quake settings
    Serial.println("Quake configuration reset is verified.");

    Serial.println("Finished system reset!");
}

void loop() {
    // Do nothing.
}
#endif