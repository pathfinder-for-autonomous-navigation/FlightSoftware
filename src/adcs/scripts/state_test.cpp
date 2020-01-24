//
// test/state_test.cpp
// ADCS
//
// Contributors:
//   Kyle Krol         kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifdef UMB_DEBUG
#define DEBUG
#endif

#include <adcs/state_controller.hpp>
#include <adcs/utl/debug.hpp>

#include <Arduino.h>
#include <i2c_t3.h>

using namespace adcs;

void setup() {
  DEBUG_init(9600) DEBUG_printlnF("Debug initialized.")

  Wire.begin(I2C_SLAVE, 0x4E);
  Wire.onReceive(umb::on_i2c_recieve);
  Wire.onRequest(umb::on_i2c_request);
}

void loop() {
  delay(100);
}
