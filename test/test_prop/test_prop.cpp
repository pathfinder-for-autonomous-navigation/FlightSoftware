#include <Arduino.h>
#include <PropulsionSystem.hpp>
#include <unity.h>

Devices::PropulsionSystem prop_system;

void setup() {
  delay(2000);
  Serial.begin(9600);
  prop_system.setup();

  UNITY_BEGIN();
  UNITY_END();
}

void loop() {}
