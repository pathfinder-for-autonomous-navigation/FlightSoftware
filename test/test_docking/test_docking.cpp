#include <Arduino.h>
#include <DockingSystem.hpp>
#include <unity.h>

Devices::DockingSystem docking_system;

void setup() {
  delay(2000);
  Serial.begin(9600);
  docking_system.setup();

  UNITY_BEGIN();
  UNITY_END();
}

void loop() {}
