//
// src/adcs/rwa_config.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol         kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

// TODO : Determine reaction wheel command transformation

#ifndef SRC_ADCS_RWA_CONFIG_HPP_
#define SRC_ADCS_RWA_CONFIG_HPP_

#include "dev/AD5254.hpp"
#include "dev/ADS1015.hpp"

#include <i2c_t3.h>
#include <lin.hpp>

namespace adcs {
namespace rwa {

/** Wire for the potentiometer. */
static i2c_t3 *const pot_wire = &Wire1;
/** Address for the potentiomter. */
static unsigned char const pot_addr = dev::AD5254::ADDR::A01;
/** Timeout in microseconds for the potentiometer. */
static unsigned long const pot_timeout = 10000;

/** Clockwise enable pin for the zeroth wheel. */
static unsigned int const wheel0_cw_pin = 26;
/** Counterclockwise enable pin for the zeroth wheel. */
static unsigned int const wheel0_ccw_pin = 39;
/** Speed servo pulse pin for the zeroth wheel. */
static unsigned int const wheel0_speed_pin = 23;
/** Clockwise enable pin for the first wheel. */
static unsigned int const wheel1_cw_pin = 10;
/** Counterclockwise enable pin for the first wheel. */
static unsigned int const wheel1_ccw_pin = 25;
/** Speed servo pulse pin for the first wheel. */
static unsigned int const wheel1_speed_pin = 22;
/** Clockwise enable pin for the second wheel. */
static unsigned int const wheel2_cw_pin = 9;
/** Counterclockwise enable pin for the second wheel. */
static unsigned int const wheel2_ccw_pin = 24;
/** Speed servo pulse pin for the second wheel. */
static unsigned int const wheel2_speed_pin = 21;

/** Wire for the zeroth wheel's ADC. */
static i2c_t3 *const adc0_wire = &Wire1;
/** Address for the zeroth wheel's ADC. */
static unsigned int const adc0_addr = dev::ADS1015::ADDR::GND;
/** Alert pin for the zeroth wheel's ADC. */
static unsigned int const adc0_alrt = 11;
/** Wire for the first wheel's ADC. */
static i2c_t3 *const adc1_wire = &Wire1;
/** Address for the first wheel's ADC. */
static unsigned int const adc1_addr = dev::ADS1015::ADDR::VDD;
/** Alert pin for the first wheel's ADC. */
static unsigned int const adc1_alrt = 12;
/** Wire for the second wheel's ADC. */
static i2c_t3 *const adc2_wire = &Wire1;
/** Address for the second wheel's ADC. */
static unsigned int const adc2_addr = dev::ADS1015::ADDR::SSCL;
/** Alert pin for the second wheel's ADC. */
static unsigned int const adc2_alrt = 16;
/** Timeout value in microseconds for the wheel's ADCs. */
static unsigned long const adcx_timeout = 10000;

/** Transforms a vector from the body frame into the commanding frame for the
 *  reaction wheels. */
static constexpr lin::Matrix3x3f body_to_rwa = {
  0.0f, -1.0f,  0.0f,
  0.0f,  0.0f, -1.0f,
  1.0f,  0.0f,  0.0f
};

/** Transforms a vector from the reaction wheel control frame to the body frame
 *  of the spacecraft. */
static constexpr lin::Matrix3x3f rwa_to_body = {
   0.0f,  0.0f, 1.0f,
  -1.0f,  0.0f, 0.0f,
   0.0f, -1.0f, 0.0f
};

static_assert(lin::fro(rwa_to_body * body_to_rwa - lin::identity<lin::Matrix3x3f>()) < 1.0e-5f, "");

}  // namespace rwa
}  // namespace adcs

#endif
