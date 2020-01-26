//
// src/adcs/scripts/mtr_test.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#include <adcs/constants.hpp>
#include <adcs/mtr.hpp>
#include <adcs/utl/logging.hpp>

#include <Arduino.h>
#include <lin.hpp>

#include <cmath>

using namespace adcs;

void setup() {
  LOG_init(9600);
  mtr::setup();
}

float moment = 0.0f;
float delta = mtr::max_moment / 15.0f;

void loop() {
  if (fabs(moment) >= mtr::max_moment) delta = -delta;
  moment += delta;
  lin::Vector3f mtr_cmd({moment, moment, moment});
  mtr::actuate(MTRMode::MTR_ENABLED, mtr_cmd, 32768.0f);
  delay(500);
}
