//
// src/adcs/scripts/smoke_test.cpp
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

#include <adcs/utl/debug.hpp>

#include <Arduino.h>

void setup() {
  DEBUG_init(9600)
}

void loop() {
  delay(2000);
  DEBUG_printlnF("Still alive! :)")
}
