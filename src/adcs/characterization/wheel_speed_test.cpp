//
// include/adcs_constants.hpp
// Common
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#include <adcs/constants.hpp>
#include <adcs/imu.hpp>
#include <adcs/rwa.hpp>
#include <adcs/utl/convert.hpp>
#include <adcs/utl/logging.hpp>

#include <Arduino.h>
#include <i2c_t3.h>
#include <lin.hpp>

#include <array>

using namespace adcs;

#define HANG while (1) delay(100);

void setup() {
  // We want to initialize the serial port even if logging is off.
  LOG_SERIAL.begin(9600);

  LOG_INFO_header
  LOG_INFO_println("Logging interface initialized with logging level "
      + String(LOG_LEVEL))

  // Initialize master I2C busses
  Wire1.begin(I2C_MASTER, 0x00, I2C_PINS_37_38, I2C_PULLUP_EXT, 400000);
  Wire2.begin(I2C_MASTER, 0x00, I2C_PINS_3_4, I2C_PULLUP_EXT, 400000);

  // Setup the imu and rwa modules
  imu::setup();
  rwa::setup();

  // Check all the required devices are functional
  if(!imu::mag1.is_functional() ||
     !imu::mag2.is_functional() ||
     !rwa::adcs[0].is_functional() ||
     !rwa::adcs[1].is_functional() ||
     !rwa::adcs[2].is_functional() ||
     !rwa::wheels[0].is_functional() ||
     !rwa::wheels[1].is_functional() ||
     !rwa::wheels[2].is_functional() ||
     !rwa::potentiometer.is_functional()) {
    LOG_ERROR_header
    LOG_ERROR_printlnF("One or more of the required devices failed initialization.")
    HANG  // Cannot perform the test
  }

  // Wheel speeds the tests will be performed at
  std::array<float> wheel_speeds = {
    50.0f, 75.0f, 100.0f, 125.0f, 150.0f, 175.0f
  };

  // Loop over wheels speed and each wheel to perform the test.
  for (auto const &wheel_speed : wheel_speeds) {

    // Wait for the wheels to ramp up to the desired speed
    rwa::actuate(RWAMode::RWA_SPEED_CTRL, lin::Vector3f({wheel_speed, wheel_speed, wheel_speed}));
    delay(5000);

    // Sync up sensor reads
    imu::mag1.read()
    imu::mag2.read()
    while (!imu::mag1.is_ready());
    while (!imu::mag2.is_ready());
    imu::mag1.read()
    imu::mag2.read()

    unsigned long const start = millis();
    unsigned long const duration = 30000;
    while (millis() - start < duration) {

      // Read the first magnetometer
      while (!imu::mag1.is_ready());
      if (!imu::mag1.is_functional()) HANG
      if (!imu::mag1.read()) HANG

      // Collect timestamp for this round of data
      // Both gyros read at 50 Hz so the timestamps offset from the actual data
      // taken should be consistant
      unsigned long const timestamp = millis();

      // Read the second magnetometer
      while (!imu::mag2.is_ready());
      if (!imu::mag2.is_functional()) HANG
      if (!imu::mag2.read()) HANG

      // Print out and process data including wheel speed

    }
  }
}

void loop() {
  delay(100);
}
