//
// src/adcs/scripts/MMC34160PJ_test.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

// TODO : Figure out why getting -32088,32763,32690
// May be worth checking the data sheet again and seeing if we have some bit
// manipulation incorect.

#include <adcs/dev/MMC34160PJ.hpp>
#include <adcs/utl/logging.hpp>

#include <i2c_t3.h>
#include <Arduino.h>
#undef abs

using namespace adcs;

dev::MMC34160PJ mag;

void setup() {
  LOG_init(9600)
  Wire2.begin(I2C_MASTER, 0x00, I2C_PINS_3_4, I2C_PULLUP_EXT, 400000);

  delay(5000); // Allow magnetometer to start up
  mag.setup(&Wire2);
  if (!mag.reset()) {
    LOG_ERROR_header
    LOG_ERROR_printlnF("Initialization failed")
    while (1);
  }
  if (!mag.calibrate()) {
    LOG_ERROR_header
    LOG_ERROR_printlnF("Error")
    while (1);
  }
}

void loop() {
  for (int i=0; i<10; i++){
    while (!mag.is_ready()){
      if (!mag.is_functional()) while (1);
    }

    if (mag.read()) {
      LOG_INFO_header
      LOG_INFO_println(String(mag.get_b_x()) + "," + String(mag.get_b_y()) + "," +
          String(mag.get_b_z()))
    } else {
      LOG_ERROR_header
      LOG_ERROR_printlnF("Read error")
    }
    delay(500);
  }
  LOG_INFO_header
  LOG_INFO_printlnF("calibration")
  delay(1000);
  if(! mag.calibrate()){
    LOG_ERROR_header
    LOG_ERROR_printlnF("calibration error")
  }
  delay(1000);
}
