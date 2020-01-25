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

#ifdef MTR_DEBUG
#define DEBUG
#endif

#include "constants.hpp"
#include "mtr.hpp"
#include "mtr_config.hpp"
#include "utl/debug.hpp"

#include <Arduino.h>
#include <array>

namespace adcs {
namespace mtr {

dev::Magnetorquer mtrs[3];

void setup() {
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
}

void actuate(unsigned char mtr_mode, lin::Vector3f mtr_cmd, float mtr_lim) {
#ifdef DEBUG
  std::array<int, 3> pwm_value = { 0, 0, 0 };
#endif
  // Account for calibration
  mtr_cmd = body_to_mtr * mtr_cmd;

  // Check if the magnetic torque rods are enabled
  if (mtr_mode == MTRMode::MTR_ENABLED) {
    // Loop over the magnetic torque rods
    for (int i = 0; i < 3; i++) {
      // Actuate enabled magnetic torque rods
      if (mtrs[i].is_functional()) {
        float command = (mtr_cmd(i) > mtr_lim ? mtr_lim : mtr_cmd(i));
        command = (mtr_cmd(i) < -mtr_lim ? -mtr_lim : mtr_cmd(i));
        mtrs[i].actuate((int)(32768.0f * command / max_moment));
#ifdef DEBUG
        pwm_value[i] = (int)(32768.0f * command / max_moment);
#endif
      }
    }
  } else {
    // Magnetic torque rods are disabled
    for (auto &mtr : mtrs) mtr.actuate(0);
  }
  DEBUG_println(String(mtr_cmd(0)) + "," + String(mtr_cmd(1)) + "," + String(mtr_cmd(2))
      + "," + String(pwm_value[0]) + "," + String(pwm_value[1]) + "," + String(pwm_value[2]))
}
}  // namespace mtr
}  // namespace adcs
