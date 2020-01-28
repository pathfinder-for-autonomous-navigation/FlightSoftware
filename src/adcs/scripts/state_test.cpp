//
// src/adcs/scripts/state_test.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol         kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#include <adcs/state_controller.hpp>
#include <adcs/utl/logging.hpp>

#include <Arduino.h>
#include <i2c_t3.h>

using namespace adcs;

void setup() {
  LOG_init(9600)
  LOG_INFO_println("Logging interface initialized")

  Wire.begin(I2C_SLAVE, 0x4E);
  Wire.onReceive(umb::on_i2c_recieve);
  Wire.onRequest(umb::on_i2c_request);
}

void loop() {
  delay(100);
}
