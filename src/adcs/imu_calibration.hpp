//
// include/imu_calibration.hpp
// ADCS
//
// Contributors:
//   Kyle Krol         kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

// TODO : Determine imu read transformations

#ifndef ADCS_INCLUDE_IMU_CALIBRATION_HPP_
#define ADCS_INCLUDE_IMU_CALIBRATION_HPP_

#include <lin.hpp>

namespace imu {

/** @fn calibrate
 *  @param[inout] omega Omega read by the gyroscope in radians per second.
 *  @param[in] temp Temperature in celcius of the gyroscope.
 *  Transforms the omega vector in the body frame of the gyroscope to perform a
 *  calibration of the sensor. */
extern void calibrate(lin::Vector3f &omega, float temp);

}  // namespace imu

#endif
