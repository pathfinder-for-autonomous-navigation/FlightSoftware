//
// src/adcs/ssa_config.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol         kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

// TODO : Consider moving this definition to the psim repository

#ifndef SRC_ADCS_SSA_CONFIG_HPP_
#define SRC_ADCS_SSA_CONFIG_HPP_

#include "dev/ADS1015.hpp"

#include <i2c_t3.h>
#include <lin.hpp>

namespace adcs {
namespace ssa {

/** Wire for the face two ADC. */
static i2c_t3 *const adc2_wire = &Wire1;
/** Address for the zeroth ADC. */
static unsigned char const adc2_addr = dev::ADS1015::ADDR::SSDA;
/** Alert pin for the zeroth ADC. */
static unsigned int const adc2_alrt = 13;
/** Wire for the face three ADC. */
static i2c_t3 *const adc3_wire = &Wire2;
/** Address for the face three ADC. */
static unsigned char const adc3_addr = dev::ADS1015::ADDR::GND;
/** Alert pin for the face three ADC. */
static unsigned int const adc3_alrt = 15;
/** Wire for the face four ADC. */
static i2c_t3 *const adc4_wire = &Wire2;
/** Address for the face four ADC. */
static unsigned char const adc4_addr = dev::ADS1015::ADDR::VDD;
/** Alert pin for the face four ADC. */
static unsigned int const adc4_alrt = 20;
/** Wire for the face five ADC. */
static i2c_t3 *const adc5_wire = &Wire2;
/** Address for the face five ADC. */
static unsigned char const adc5_addr = dev::ADS1015::ADDR::SSDA;
/** Alert pin for the face five ADC. */
static unsigned int const adc5_alrt = 27;
/** Wire for the face six ADC. */
static i2c_t3 *const adc6_wire = &Wire2;
/** Address for the face six ADC. */
static unsigned char const adc6_addr = dev::ADS1015::ADDR::SSCL;
/** Alert pin for the six five ADC. */
static unsigned int const adc6_alrt = 28;
/** Timeout in microseconds for the ADCs. */
static unsigned long const adcx_timeout = 10000;

/** Required number of sensors in view of the sun to calculate the sun vector
 *  via least squares. */
static unsigned int const sensor_count_thresh = 4;
/** Voltage threshold for a sun sensor to be considered in view of the sun. */
static float const sensor_voltage_thresh = 1.0f;

static constexpr float s20 = std::cos(20.0);
static constexpr float c20 = std::sin(20.0);
/** Matrix containing the normal vectors for all sun sensors in the ADCS system.
 *  Should be defined for the leader and follower spacecraft individually. */
static const lin::Matrix<float, 20, 3> normals({
  // Face 2 (+x)
   c20,  0.f,  s20,
   c20,  0.f, -s20,
   c20, -s20,  0.f,
   c20,  s20,  0.f,
  // Face 3 (-y)
  -s20, -c20,  0.f,
   s20, -c20,  0.f,
   0.f, -c20, -s20,
   0.f, -c20,  s20,
  // Face 4 (-x)
  -c20,  s20,  0.f,
  -c20, -s20,  0.f,
  -c20,  0.f, -s20,
  -c20,  0.f,  s20,
  // Face 5 (+y)
   s20,  c20,  0.f,
  -s20,  c20,  0.f,
   0.f,  c20, -s20,
   0.f,  c20,  s20,
  // Face 6 (+z)
  -s20,  0.f,  c20,
   s20,  0.f,  c20,
   0.f, -s20,  c20,
   0.f,  s20,  c20
});
}  // namespace ssa
}  // namespace adcs

#endif
