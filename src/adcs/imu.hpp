//
// src/adcs/imu.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_IMU_HPP_
#define SRC_ADCS_IMU_HPP_

#include "dev/LIS2MDLTR.hpp"
#include "dev/LSM6DSM.hpp"
#include "dev/MMC34160PJ.hpp"

#include <i2c_t3.h>
#include <lin.hpp>

namespace adcs {
namespace imu {

/** Magnetometer number one device. */
extern dev::LIS2MDLTR mag1;

/** Magnetometer number two device. */
extern dev::MMC34160PJ mag2;

/** Current magnetic field reading in Tesla in the body frame of the
 *  spacecraft. */
extern lin::Vector3f mag_rd;

/** Gyroscope device. */
extern dev::LSM6DSM gyr;

/** Current gyroscope reading in radians per second in the body frame of the
 *  spacecraft. */
extern lin::Vector3f gyr_rd;

/** Current gyroscope temperature reading in degrees Celcius. */
extern float gyr_temp_rd;

/** @fn setup
 *  Initializes the magnetomters and gyroscope IC including calibration. This
 *  will take a few hundred milliseconds to complete. */
void setup();

/** @fn update_sensors
 *  @return Updated IMU assembly mode.
 *  See the documentation on OneDrive for more details about how the system
 *  behaves according to the mode value. */
unsigned char update_sensors(unsigned char mode, float mag_flt, float gyr_flt,
    float gyr_temp_eq, float gyr_temp_flt, float gry_temp_k_p,
    float gyr_temp_k_i, float gyr_temp_k_d);

}  // namespace imu
}  // namespace adcs

#endif
