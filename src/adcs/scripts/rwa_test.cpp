//
// src/adcs/scripts/rwa_test.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

/* Runs a single and multicomponent test of the reaction wheel system. Note that
 * this script needs to be compiled with the RWA_DEBUG flag to get the CSV
 * output.
 * See plot/rwaPlot.py to interpret the CSV file. */

#include <adcs/constants.hpp>
#include <adcs/rwa.hpp>
#include <adcs/utl/logging.hpp>

#include <Arduino.h>
#include <i2c_t3.h>
#include <lin.hpp>

using namespace adcs;

#define MAX_TORQUE rwa::max_torque
#define SPEED_FLT 0.75f
#define RAMP_FLT 0.75f

/** @fn take_data
 *  Takes reaction wheel data and prints out a timestamped CSV line. */
void take_data() {
  LOG_INFO_print(String(millis() - 10000) + ",")
  rwa::update_sensors(SPEED_FLT, RAMP_FLT);
  LOG_INFO_println()
}

/** @fn single_component_test
 *  Performs a ramp up and down single component test on the specified reaction
 *  wheel.
 *  @param[in] wheel Wheel the test is performed on. */
void single_component_test(unsigned int wheel) {
  rwa::wheels[wheel].reset();
  lin::Vector3f torque = lin::zeros<float, 3, 1>();
  // Spin the wheel
  torque(wheel) = MAX_TORQUE;
  rwa::actuate(RWAMode::RWA_ACCEL_CTRL, torque);
  for (unsigned int i = 0; i < 150; i++) {
    take_data();
    delay(20);
  }
  // Spin wheel down through zero
  torque(wheel) = -MAX_TORQUE;
  rwa::actuate(RWAMode::RWA_ACCEL_CTRL, torque);
  for (unsigned int i = 0; i < 300; i++) {
    take_data();
    delay(20);
  }
  // Spin wheel back up
  torque(wheel) = MAX_TORQUE;
  rwa::actuate(RWAMode::RWA_ACCEL_CTRL, torque);
  for (unsigned int i = 0; i < 150; i++) {
    take_data();
    delay(20);
  }
  // Set back to zero torque
  torque(wheel) = 0.0f;
  rwa::actuate(RWAMode::RWA_ACCEL_CTRL, torque);
  rwa::wheels[wheel].disable();
}

void setup() {
  LOG_init(9600)
  Wire1.begin(I2C_MASTER, 0x00, I2C_PINS_37_38, I2C_PULLUP_EXT, 400000);
  delay(10000);
  rwa::setup();

  // rwa::actuate(RWAMode::RWA_ACCEL_CTRL, {255.0f, 255.0f, 255.0f});

  /* Single component tests */
  for (auto &wheel : rwa::wheels) wheel.disable();
  single_component_test(0);
  single_component_test(1);
  single_component_test(2);
  for (unsigned int i = 0; i < 10; i++) {
    take_data();
    delay(20);
  }
  for (auto &wheel : rwa::wheels) wheel.reset();

  /* Multicomponent tests */
  lin::Vector3f torque({MAX_TORQUE / 10.0f, MAX_TORQUE / 10.0f, MAX_TORQUE / 10.0f});
  for (unsigned int i = 0; i < 10; i++) {
    float component = ((float)(i + 1)) * MAX_TORQUE / 10.0f;
    lin::Vector3f torque({component, component, component});
    rwa::actuate(RWAMode::RWA_ACCEL_CTRL, torque);
    for (unsigned int j = 0; j < 25; j++) {
      take_data();
      delay(40);
    }
    torque = {-component, -component, -component};
    rwa::actuate(RWAMode::RWA_ACCEL_CTRL, torque);
    for (unsigned int j = 0; j < 25; j++) {
      take_data();
      delay(40);
    }
  }

  /* Disable wheels */
  for (auto &wheel : rwa::wheels) wheel.disable();

}

void loop() {
  delay(1);
}
