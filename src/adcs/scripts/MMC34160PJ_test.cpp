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

#define DEBUG

#include <adcs/dev/MMC34160PJ.hpp>
#include <adcs/utl/debug.hpp>

#include <i2c_t3.h>
#include <Arduino.h>

using namespace adcs;

dev::MMC34160PJ mag;

void setup() {
  DEBUG_init(9600)
  Wire2.begin(I2C_MASTER, 0x00, I2C_PINS_3_4, I2C_PULLUP_EXT, 400000);

  delay(5000); // Allow magnetometer to start up
  DEBUG_printlnF("Starting")
  mag.setup(&Wire2);
  if (!mag.reset()) {
    DEBUG_printlnF("Error")
    while (1);
  }
}

void loop() {

  while (!mag.is_ready())
    if (!mag.is_functional()) while (1);

  if (mag.read()) {
    DEBUG_println(String(mag.get_b_x()) + "," + String(mag.get_b_y()) + "," +
        String(mag.get_b_z()))
  } else {
    DEBUG_printlnF("Error")
  }

  delay(2000);
  
}
