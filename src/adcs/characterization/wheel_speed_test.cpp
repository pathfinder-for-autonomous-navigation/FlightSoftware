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

  LOG_INFO_header
  LOG_INFO_printlnF("Waiting for ten seconds before starting the test...")
  delay(15000);

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
  std::array<float, 10> wheel_speeds = {
    5.0f, 10.0f, 15.0f, 20.0f, 25.0f, 30.0f, 35.0f, 40.0f, 45.0f, 50.0f
  };

  // Loop over wheels speed and each wheel to perform the test.
  for (auto const &wheel_speed : wheel_speeds) {

    // Wait for the wheels to ramp up to the desired speed
    rwa::wheels[0].disable();
    rwa::wheels[1].disable();
    rwa::actuate(RWAMode::RWA_SPEED_CTRL, lin::Vector3f({wheel_speed, wheel_speed, wheel_speed}));
    delay(5000);

    // Sync up sensor reads
    imu::mag1.read();
    imu::mag2.read();
    while (!imu::mag1.is_ready());
    while (!imu::mag2.is_ready());
    imu::mag1.read();
    imu::mag2.read();

    unsigned long const start = millis();
    unsigned long const duration = 10000;
    while (millis() - start < duration) {

      // Read the first magnetometer
      while (!imu::mag1.is_ready());
      if (!imu::mag1.is_functional()) HANG
      if (!imu::mag1.read()) HANG

      // Read the second magnetometer
      while (!imu::mag2.is_ready());
      if (!imu::mag2.is_functional()) HANG
      if (!imu::mag2.read()) HANG

      // Initialize read for each enabled ADC
      int16_t val;
      lin::Vector3f speed_read;
      for (unsigned int i = 0; i < 3; i++)
        if (rwa::adcs[i].is_functional())
          rwa::adcs[i].start_read(dev::ADS1015::CHANNEL::DIFFERENTIAL_0_1);

      // End read for each enabled ADC
      for (unsigned int i = 0; i < 3; i++)
        if (rwa::adcs[i].is_functional())
          if (rwa::adcs[i].end_read(val))
            speed_read(i) = utl::fp(val, rwa::min_speed_read, rwa::max_speed_read);

      LOG_SERIAL.printf("%d,", millis());
      LOG_SERIAL.printf("%.7e,", utl::fp(imu::mag1.get_b_x(), imu::min_mag1_rd_mag, imu::max_mag1_rd_mag));
      LOG_SERIAL.printf("%.7e,", utl::fp(imu::mag1.get_b_y(), imu::min_mag1_rd_mag, imu::max_mag1_rd_mag));
      LOG_SERIAL.printf("%.7e,", utl::fp(imu::mag1.get_b_z(), imu::min_mag1_rd_mag, imu::max_mag1_rd_mag));
      LOG_SERIAL.printf("%.7e,", utl::fp(imu::mag2.get_b_x(), imu::min_mag2_rd_mag, imu::max_mag2_rd_mag));
      LOG_SERIAL.printf("%.7e,", utl::fp(imu::mag2.get_b_y(), imu::min_mag2_rd_mag, imu::max_mag2_rd_mag));
      LOG_SERIAL.printf("%.7e,", utl::fp(imu::mag2.get_b_z(), imu::min_mag2_rd_mag, imu::max_mag2_rd_mag));
      LOG_SERIAL.printf("%.7e,", speed_read(0));
      LOG_SERIAL.printf("%.7e,", speed_read(1));
      LOG_SERIAL.printf("%.7e,", speed_read(2));
      LOG_SERIAL.println();
    }
  }

  // Trigger the python script to stop taking data
  LOG_SERIAL.println("#COMPLETE");

  // Shut the wheels off
  rwa::actuate(RWAMode::RWA_SPEED_CTRL, lin::zeros<lin::Vector3f>());
  delay(5000); 
  rwa::wheels[0].disable();
  rwa::wheels[1].disable();
  rwa::wheels[2].disable();
}

void loop() {
  delay(100);
}
