//
// src/adcs/ssa.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol         kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

// TODO :
// Determine sun sensor normal vectors
// Determine voltage threshold for inclusion in least squares
// Determine least squares row threshold
// Review timeout value

#ifndef SRC_ADCS_SSA_HPP_
#define SRC_ADCS_SSA_HPP_

#include "dev/ADS1015.hpp"

#include <lin.hpp>

namespace adcs {
namespace ssa {

/** ADC device array. */
extern dev::ADS1015 adcs[5];

/** Matrix storing sun sensor voltage readings. Row position indicates ADC
 *  membership and columns map to channel number. */
extern lin::Matrix<float, 5, 4> voltages;

/** @fn setup
 *  Initiates all the ADCs but does not initialize voltage values in the
 *  in the voltage vector. */
void setup();

/** @fn actuate
 *  Takes a round of sensor readings with the given exponential filter constant.
 *  The new voltages can be accessed via the extern voltages vector.
 *  @param[in] adc_flt Exponential filter applied to the voltage readings. */
void update_sensors(float adc_flt);

/** @fn calculate
 *  Determines a sun vector given the current voltage readings. If an accurate
 *  sun vector cannot be determined at the current time, the function will
 *  return FAILURE instead of COMPLETE.
 *  @param[out] sun_vec Normalized vector in R3 is written to this reference.
 *  @return Sun sensor resulting mode - i.e. COMPLETE or FAILURE. */
unsigned char calculate_sun_vector(lin::Vector3f &sun_vec);

}  // namespace ssa
}  // namespace adcs

#endif
