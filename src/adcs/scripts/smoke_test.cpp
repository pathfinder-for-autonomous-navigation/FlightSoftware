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

#define LOG_LEVEL 99

#include <adcs/utl/logging.hpp>

#include <Arduino.h>

void setup() {
  LOG_init(9600)
}

void loop() {
  delay(2000);
  LOG_TRACE_printlnF("Still alive! :)")
}
