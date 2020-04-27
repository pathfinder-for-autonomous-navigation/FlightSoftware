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
#include "dev/GyroHeaterTransistor.hpp"

#include <i2c_t3.h>
#include <lin.hpp>

namespace adcs {
namespace imu {

/** Magnetometer number one device. */
extern dev::LIS2MDLTR mag1;

/** Current magnetic field reading in Tesla in the body frame of the
 *  spacecraft from magnetomter one. */
extern lin::Vector3f mag1_rd;

/** Magnetometer number two device. */
extern dev::MMC34160PJ mag2;

/** Current magnetic field reading in Tesla in the body frame of the
 *  spacecraft from magnetomter two. */
extern lin::Vector3f mag2_rd;

/** Gyroscope device. */
extern dev::LSM6DSM gyr;

/** Gyro heater device */
extern dev::GyroHeaterTransistor gyr_heater;

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
void update_sensors(unsigned char mag1_mode, unsigned char mag2_mode,
    float mag_flt, float gyr_flt, float gyr_temp_eq, float gyr_temp_flt,
    unsigned char gyr_temp_pwm);

}  // namespace imu
}  // namespace adcs

#endif
