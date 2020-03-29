//
// src/adcs/rwa.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

// TODO:
//  - transform sensor readings in to the body frame

#ifdef RWA_LOG_LEVEL
  #undef LOG_LEVEL
  #define LOG_LEVEL RWA_LOG_LEVEL
#endif

#include "constants.hpp"
#include "rwa.hpp"
#include "rwa_config.hpp"
#include "utl/convert.hpp"
#include "utl/logging.hpp"

#include <lin/generators/constants.hpp>

namespace adcs {
namespace rwa {

dev::MaxonEC45 wheels[3];

dev::ADS1015 adcs[3];

dev::AD5254 potentiometer;

void setup() {
  LOG_INFO_header
  LOG_INFO_printlnF("Initializing the RWA module")

  // Setup and reset the potentiometer
  potentiometer.setup(pot_wire, pot_addr, pot_timeout);
  potentiometer.reset();

#if LOG_LEVEL >= LOG_LEVEL_ERROR
  if (!potentiometer.is_functional()) {
    LOG_ERROR_header
    LOG_ERROR_printlnF("Potentiometer initialization failed")
  }
#endif

  // Setup the alert pins to input mode
  pinMode(adc0_alrt, INPUT);
  pinMode(adc1_alrt, INPUT);
  pinMode(adc2_alrt, INPUT);

  // Setup the ADCs on the above pins
  adcs[0].setup(adc0_wire, adc0_addr, adc0_alrt, adcx_timeout);
  adcs[1].setup(adc1_wire, adc1_addr, adc1_alrt, adcx_timeout);
  adcs[2].setup(adc2_wire, adc2_addr, adc2_alrt, adcx_timeout);

  // Reset the ADCs and set their gain to one
  for (auto &adc : adcs) {
    adc.set_gain(dev::ADS1015::GAIN::ONE);
    adc.reset();
  }

#if LOG_LEVEL >= LOG_LEVEL_ERROR
  for (unsigned int i = 0; i < 3; i++) {
    if (!adcs[i].is_functional()) {
      LOG_ERROR_header
      LOG_ERROR_println("ADC" + String(i) + " initialization failed")
    }
  }
#endif

  // Initiate reaction wheel pins to output
  pinMode(wheel0_cw_pin, OUTPUT);
  pinMode(wheel0_ccw_pin, OUTPUT);
  pinMode(wheel1_cw_pin, OUTPUT);
  pinMode(wheel1_ccw_pin, OUTPUT);
  pinMode(wheel2_cw_pin, OUTPUT);
  pinMode(wheel2_ccw_pin, OUTPUT);

  // Setup the wheels on the above pins
  wheels[0].setup(wheel0_cw_pin, wheel0_ccw_pin, wheel0_speed_pin, &potentiometer, &dev::AD5254::set_rdac0);
  wheels[1].setup(wheel1_cw_pin, wheel1_ccw_pin, wheel1_speed_pin, &potentiometer, &dev::AD5254::set_rdac1);
  wheels[2].setup(wheel2_cw_pin, wheel2_ccw_pin, wheel2_speed_pin, &potentiometer, &dev::AD5254::set_rdac2);

  // Reset each wheel
  for (auto &wheel : wheels) wheel.reset();

  LOG_INFO_header
  LOG_INFO_printlnF("Complete")
}

lin::Vector3f speed_rd = lin::zeros<float, 3, 1>();
lin::Vector3f ramp_rd = lin::zeros<float, 3, 1>();

void update_sensors(float speed_flt, float ramp_flt) {
  int16_t val;
  lin::Vector3f readings;

  LOG_TRACE_header
  LOG_TRACE_printlnF("Updating RWA sensors")

  // Retain value on isolated error
  readings = speed_rd;

  // Initialize read for each enabled ADC
  for (unsigned int i = 0; i < 3; i++)
    if (adcs[i].is_functional())
      adcs[i].start_read(dev::ADS1015::CHANNEL::DIFFERENTIAL_0_1);

  // End read for each enabled ADC
  for (unsigned int i = 0; i < 3; i++)
    if (adcs[i].is_functional())
      if (adcs[i].end_read(val))
        readings(i) = 4.096f * ((float)val) / 2048.0f;
        // ^^ Only converting to a voltage here

  // Go from voltage to speed reading
  readings = (max_speed_read - min_speed_read) * readings / 3.3f +
      min_speed_read * lin::ones<lin::Vector3f>();

  // Filter the results
  speed_rd = speed_rd + speed_flt * (readings - speed_rd);

  LOG_TRACE_header
  LOG_TRACE_println("Updated speed reading " + String(readings(0)) + " "
      + String(readings(1)) + " " + String(readings(2)))

  LOG_TRACE_header
  LOG_TRACE_println("Updated, filtered speed reading "
      + String(speed_rd(0)) + " " + String(speed_rd(1)) + " "
      + String(speed_rd(2)))

  // Retain value on isolated error
  readings = ramp_rd;

  // Begin read for each enabled ADC
  for (unsigned int i = 0; i < 3; i++)
    if (adcs[i].is_functional())
      adcs[i].start_read(dev::ADS1015::CHANNEL::SINGLE_2);

  // End read for each enabled ADC
  for (unsigned int i = 0; i < 3; i++)
    if (adcs[i].is_functional())
      if (adcs[i].end_read(val))
        readings(i) = 4.096f * ((float)val) / 2048.0f;
        // ^^ Only converting to a voltage here
        
  // Go from voltage to torque reading
  readings = (max_torque - min_torque) * readings / 3.3f +
      min_torque * lin::ones<lin::Vector3f>();

  // Filter the results
  ramp_rd = ramp_rd + speed_flt * (readings - ramp_rd);

  LOG_TRACE_header
  LOG_TRACE_println("Updated ramp reading " + String(readings(0)) + " "
      + String(readings(1)) + " " + String(readings(2)))

  LOG_TRACE_header
  LOG_TRACE_println("Updated, filtered ramp reading "
      + String(ramp_rd(0)) + " " + String(ramp_rd(1)) + " "
      + String(ramp_rd(2)))
}

void actuate(unsigned char rwa_mode, lin::Vector3f rwa_cmd) {
  LOG_TRACE_header
  LOG_TRACE_printlnF("Actuatings reaction wheels")

  // Account for calibrations
  rwa_cmd = body_to_rwa * rwa_cmd;

  LOG_TRACE_header
  LOG_TRACE_println("Commands wheels in mode " + String(rwa_mode) + " to"
      + String(rwa_cmd(0)) + " " + String(rwa_cmd(1)) + " "
      + String(rwa_cmd(2)))

  switch (rwa_mode) {

    case RWAMode::RWA_ACCEL_CTRL: {
      // Ensure working potentiometer for acceleration control
      if (!potentiometer.is_functional()) break;

      // Set wheel configurations
      for (unsigned int i = 0; i < 3; i++) {
        if (rwa_cmd(i) >= 0) {
          wheels[i].set_speed(2000);
          wheels[i].set_axl_ramp(utl::uc(rwa_cmd(i), 0.0f, rwa::max_torque));
        } else {
          wheels[i].set_speed(-2000);
          wheels[i].set_axl_ramp(utl::uc(-rwa_cmd(i), 0.0f, rwa::max_torque));
        }
      }

      // Perform actuation
      for (auto &wheel : wheels)
        if (wheel.is_functional()) wheel.actuate();
      potentiometer.write_rdac();

      break;
    }

    case RWAMode::RWA_SPEED_CTRL: {
      // Set wheel configurations
      for (unsigned int i = 0; i < 3; i++) {
        if (wheels[i].is_functional()) {
          wheels[i].set_axl_ramp(255);
          wheels[i].set_speed((int)(1000.0f * rwa_cmd(i) / rwa::max_speed_command + 1000.0f));
        }
      }

      // Actuate wheel if functional
      for (auto &wheel : wheels)
        if (wheel.is_functional()) wheel.actuate();
      if (potentiometer.is_functional()) potentiometer.write_rdac();

      break;
    }

    // Default/disabled
    default: {
      for (auto &wheel : wheels)
        if (wheel.is_functional()) wheel.stop();
    }
  }

  LOG_TRACE_header
  LOG_TRACE_printlnF("Complete")
}
}  // namespace rwa
}  // namespace adcs
