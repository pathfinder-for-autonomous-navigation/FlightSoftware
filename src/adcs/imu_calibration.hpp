//
// src/adcs/imu_calibration.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol         kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

// TODO : Determine imu read transformations

#ifndef SRC_ADCS_IMU_CALIBRATION_HPP_
#define SRC_ADCS_IMU_CALIBRATION_HPP_

#include <lin.hpp>

namespace adcs {
namespace imu {

/** @fn calibrate
 *  @param[inout] omega Omega read by the gyroscope in radians per second.
 *  @param[in] temp Temperature in celcius of the gyroscope.
 *  Transforms the omega vector in the body frame of the gyroscope to perform a
 *  calibration of the sensor. */
void calibrate(lin::Vector3f &omega, float temp);

}  // namespace imu
}  // namespace adcs

#endif
