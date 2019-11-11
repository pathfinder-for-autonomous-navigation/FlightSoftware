#include <unity.h>
#include <Arduino.h>
#include <DockingSystem.hpp>

Devices::DockingSystem docking_system;

void setup() {
    delay(2000);
    Serial.begin(9600);
    docking_system.setup();

    UNITY_BEGIN();
    UNITY_END();
}

void loop() {

}
