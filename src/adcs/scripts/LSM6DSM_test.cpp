//
// src/adcs/scripts/LSM6DSM_test.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#include <adcs/dev/LSM6DSM.hpp>
#include <adcs/utl/logging.hpp>

#include <Arduino.h>
#include <i2c_t3.h>

using namespace adcs;

dev::LSM6DSM gyro;

void setup() {
  LOG_init(9600)

  Wire2.begin(I2C_MASTER, 0x00, I2C_PINS_3_4, I2C_PULLUP_EXT, 400000);
  delay(5000); // Allow gyroscope to start up

  gyro.setup(&Wire2, dev::LSM6DSM::ADDR::GND, 10000);
  if (gyro.reset()) {
    LOG_INFO_header
    LOG_INFO_printlnF("Gyroscope initialized")
  } else {
    LOG_ERROR_header
    LOG_ERROR_printlnF("Gyroscope initialization failed")
    while (1);
  }
}

unsigned long time_stamp = 0;

void loop() {

  while (!gyro.is_ready()) delay(2);

  if (gyro.read()) {
    if (millis() - 2000 > time_stamp) {
      time_stamp = millis();

      LOG_INFO_header
      LOG_INFO_println("Omega " + String(gyro.get_omega_x()) + " "
          + String(gyro.get_omega_y()) + " " + String(gyro.get_omega_z()))

      LOG_INFO_header
      LOG_INFO_println("temp " + String(gyro.get_temp()))
    }
  } else {
    LOG_ERROR_header
    LOG_ERROR_printlnF("Gyroscope read failed")
  }
}
