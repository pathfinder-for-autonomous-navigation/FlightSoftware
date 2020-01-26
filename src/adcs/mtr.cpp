//
// src/adcs/mtr.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

// TODO :
// Determine the magnetic moment to PWM scaling factor.
// Ensure pin assignments match with the body frame of the spacecraft

#ifdef MTR_LOG_LEVEL
  #define LOG_LEVEL MTR_LOG_LEVEL
#endif

#include "constants.hpp"
#include "mtr.hpp"
#include "mtr_config.hpp"
#include "utl/logging.hpp"

#include <Arduino.h>

namespace adcs {
namespace mtr {

dev::Magnetorquer mtrs[3];

void setup() {
  LOG_INFO_header
  LOG_INFO_printlnF("Initializing the MTR module")

  // Set the PWM resolution and frequency
  analogWriteResolution(15);
  analogWriteFrequency(mtr0_f_pin, 1831.055f);

  // Configure PWM pins for output
  pinMode(mtr0_f_pin, OUTPUT);
  pinMode(mtr0_r_pin, OUTPUT);
  pinMode(mtr1_f_pin, OUTPUT);
  pinMode(mtr1_r_pin, OUTPUT);
  pinMode(mtr2_f_pin, OUTPUT);
  pinMode(mtr2_r_pin, OUTPUT);

  // Setup the magnetorquers on the above pins
  mtrs[0].setup(mtr0_f_pin, mtr0_r_pin);
  mtrs[1].setup(mtr1_f_pin, mtr1_r_pin);
  mtrs[2].setup(mtr2_f_pin, mtr2_r_pin);

  // Reset the magnetorquers
  for (auto &mtr : mtrs) mtr.reset();

  LOG_INFO_header
  LOG_INFO_println("MTR module initialization complete")
}

void actuate(unsigned char mtr_mode, lin::Vector3f mtr_cmd, float mtr_lim) {
  LOG_TRACE_header
  LOG_TRACE_println("Actuating MTRs")

  // Account for calibration
  mtr_cmd = body_to_mtr * mtr_cmd;

  LOG_TRACE_header
  LOG_TRACE_println("Commanding MTRs to " + String(mtr_cmd(0)) + " "
      + String(mtr_cmd(1)) + " " + String(mtr_cmd(2)))

  // Check if the magnetic torque rods are enabled
  if (mtr_mode != MTRMode::MTR_ENABLED) {
    for (auto &mtr : mtrs)
      if (mtr.is_functional()) mtr.actuate(0);
    return;
  }

  LOG_TRACE_header
  LOG_TRACE_printF("Commanding MTR PWMs to")

  // Actuate the magnetic torque rods
  for (int i = 0; i < 3; i++) {
    // Ensure each magnetic torque rod is functional
    if (!mtrs[i].is_functional()) {
      LOG_TRACE_printF(" disabled")
      continue;
    }

    // Clamp the command
    float cmd = (mtr_cmd(i) > mtr_lim ? mtr_lim : mtr_cmd(i));
    cmd = (mtr_cmd(i) < -mtr_lim ? -mtr_lim : mtr_cmd(i));

    mtrs[i].actuate((int)(32768.0f * cmd / max_moment));

    LOG_TRACE_print(" " + String((int)(32768.0f * cmd / max_moment)))
  }

  // Finally end the logging line
  LOG_TRACE_println()

  LOG_TRACE_header
  LOG_TRACE_printlnF("Complete")
}
}  // namespace mtr
}  // namespace adcs
