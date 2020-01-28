//
// src/adcs/scripts/imu_test.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

// TODO
// I suspect a floating point formatting issue with `String((float) x)` that
// shows the magnetometers readings zero frequently

#include <adcs/constants.hpp>
#include <adcs/imu.hpp>
#include <adcs/utl/logging.hpp>

#include <Arduino.h>

using namespace adcs;

void setup() {
  LOG_init(9600)
  Wire1.begin(I2C_MASTER, 0x00, I2C_PINS_37_38, I2C_PULLUP_EXT, 400000);
  Wire2.begin(I2C_MASTER, 0x00, I2C_PINS_3_4, I2C_PULLUP_EXT, 400000);
  delay(5000);
  imu::setup();
}

void loop() {
  delay(1000);
  imu::update_sensors(
      IMUMode::MAG2,  // IMU mode
      0.9f,           // MAG filter
      0.9f,           // GYR filter
      30.0f,          // GYR temp equilibrium
      0.9f,           // GYR temp filter
      0.0f,           // Kp
      0.0f,           // Ki
      0.0f            // Kd
    );
}
