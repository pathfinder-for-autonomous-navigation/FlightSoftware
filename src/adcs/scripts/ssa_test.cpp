//
// src/adcs/scripts/ssa_test.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#define DEBUG

#include <adcs/ssa.hpp>
#include <adcs/utl/logging.hpp>

#include <Arduino.h>
#include <i2c_t3.h>

using namespace adcs;

void setup() {
  LOG_init(9600)
  Wire1.begin(I2C_MASTER, 0x00, I2C_PINS_37_38, I2C_PULLUP_EXT, 400000);
  Wire2.begin(I2C_MASTER, 0x00, I2C_PINS_3_4, I2C_PULLUP_EXT, 400000);
  ssa::setup();
}

void loop() {
  unsigned long time = millis();
  ssa::update_sensors(0.85f);

  LOG_INFO_header
  LOG_INFO_println("total update time " + String(millis() - time))
  LOG_INFO_println()
  delay(2000);
}
