//
// include/imu.hpp
// ADCS
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef PAN_ADCS_INCLUDE_IMU_HPP_
#define PAN_ADCS_INCLUDE_IMU_HPP_

#include <i2c_t3.h>
#include <lin.hpp>
#include <LIS2MDLTR.hpp>
#include <LSM6DSM.hpp>
#include <MMC34160PJ.hpp>

/** @namespace imu
 *  Holds all constants and functions directly related to the IMU assembly
 *  within the ADCS system. This includes the two magnetomters, gyroscope, and
 *  temperature feedback control system for the gyroscope. */
namespace imu {

/** Magnetometer number one device. */
extern LIS2MDLTR mag1;

/** Magnetometer number two device. */
extern MMC34160PJ mag2;

/** Current magnetic field reading in Tesla in the body frame of the
 *  spacecraft. */
extern lin::Vector3f mag_rd;

/** Gyroscope device. */
extern LSM6DSM gyr;

/** Current gyroscope reading in radians per second in the body frame of the
 *  spacecraft. */
extern lin::Vector3f gyr_rd;

/** Current gyroscope temperature reading in degrees Celcius. */
extern float gyr_temp_rd;

/** @fn setup
 *  Initializes the magnetomters and gyroscope IC including calibration. This
 *  will take a few hundred milliseconds to complete. */
extern void setup();

/** @fn update_sensors
 *  @return Updated IMU assembly mode.
 *  See the documentation on OneDrive for more details about how the system
 *  behaves according to the mode value. */
extern unsigned char update_sensors(unsigned char mode, float mag_flt,
    float gyr_flt, float gyr_temp_eq, float gyr_temp_flt, float gry_temp_k_p,
    float gyr_temp_k_i, float gyr_temp_k_d);

}  // namespace imu

#endif
