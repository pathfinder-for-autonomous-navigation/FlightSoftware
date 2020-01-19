//
// test/mtr_test.cpp
// ADCS
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#define DEBUG

#include <mtr.hpp>
#include <utl/debug.hpp>

#include <adcs_constants.hpp>
#include <Arduino.h>
#include <lin.hpp>

#include <cmath>

void setup() {
  DEBUG_init(9600);
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
