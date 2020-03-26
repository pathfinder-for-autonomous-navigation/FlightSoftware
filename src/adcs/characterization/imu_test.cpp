//
// src/characterization/imu_test.cpp
// adcs
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
#include <adcs/utl/convert.hpp>
#include <adcs/utl/logging.hpp>
#include <Arduino.h>
#include <i2c_t3.h>
using namespace adcs;
#define HANG while (1) delay(100);
void setup() {
  // We want to initialize the serial port even if logging is off.
  LOG_SERIAL.begin(9600);
  LOG_INFO_header
  LOG_INFO_println("Logging interface initialized with logging level "
      + String(LOG_LEVEL))
  LOG_INFO_header
  LOG_INFO_printlnF("Waiting for fifteen seconds before starting the test...")
  delay(15000);
  // Initialize master I2C busses
  Wire1.begin(I2C_MASTER, 0x00, I2C_PINS_37_38, I2C_PULLUP_EXT, 400000);
  Wire2.begin(I2C_MASTER, 0x00, I2C_PINS_3_4, I2C_PULLUP_EXT, 400000);
  // Setup the imu module
  imu::setup();
  // Check all the required devices are functional
  if(!imu::mag1.is_functional() ||
     // !imu::mag2.is_functional() || // Busted on EDU sat
     !imu::gyr.is_functional()) {
    LOG_ERROR_header
    LOG_ERROR_printlnF("One or more of the required devices failed initialization.")
    HANG  // Cannot perform the test
  }
}
void loop() {
  // Take data for about 
  for (unsigned int i = 0; i < 250; i++) {
    // Read the first magnetometer
    while (!imu::mag1.is_ready());
    if (!imu::mag1.is_functional()) HANG
    if (!imu::mag1.read()) HANG
    // Read the gyroscope
    while (!imu::gyr.is_ready());
    if (!imu::gyr.is_functional()) HANG
    if (!imu::gyr.read()) HANG
    LOG_SERIAL.printf("%d,", millis());
    LOG_SERIAL.printf("%.7e,", utl::fp(imu::mag1.get_b_x(), imu::min_mag1_rd_mag, imu::max_mag1_rd_mag));
    LOG_SERIAL.printf("%.7e,", utl::fp(imu::mag1.get_b_y(), imu::min_mag1_rd_mag, imu::max_mag1_rd_mag));
    LOG_SERIAL.printf("%.7e,", utl::fp(imu::mag1.get_b_z(), imu::min_mag1_rd_mag, imu::max_mag1_rd_mag));
    LOG_SERIAL.printf("%.7e,", utl::fp(imu::gyr.get_omega_x(), imu::max_rd_omega, imu::min_rd_omega));
    LOG_SERIAL.printf("%.7e,", utl::fp(imu::gyr.get_omega_y(), imu::max_rd_omega, imu::min_rd_omega));
    LOG_SERIAL.printf("%.7e,", utl::fp(imu::gyr.get_omega_z(), imu::max_rd_omega, imu::min_rd_omega));
    LOG_SERIAL.printf("%d", imu::gyr.get_temp());
    LOG_SERIAL.println();
  }
  // Delay for 5 minutes
  delay(5 * 60 * 1000);
}