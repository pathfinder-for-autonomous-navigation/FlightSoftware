//
// src/adcs/imu_config.hpp
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

#ifndef SRC_ADCS_IMU_CONFIG_HPP_
#define SRC_ADCS_IMU_CONFIG_HPP_

#include "dev/LSM6DSM.hpp"

#include <i2c_t3.h>
#include <lin.hpp>

namespace adcs {
namespace imu {

/** i2c bus the gyroscope communicate on. */
static i2c_t3 *const gyr_wire = &Wire2;
/** i2c address of the gyroscope. */
static unsigned char const gyr_addr = dev::LSM6DSM::ADDR::GND;
/** i2c timeout in microseconds used by the gyroscope. */
static unsigned long const gyr_timeout = 10000;

/** i2c bus the first magnetometer communicate on. */
static i2c_t3 *const mag1_wire = &Wire1;
/** i2c timeout in microseconds used by the first magnetometer. */
static unsigned long const mag1_timeout = 10000;

/** i2c bus the second magnetometer communicate on. */
static i2c_t3 *const mag2_wire = &Wire2;
/** i2c timout in microseconds used by the second magnetometer. */
static unsigned long const mag2_timeout = mag1_timeout;

/** Transformation matrix from the gyroscope's coordinate system to the body
 *  frame of the spacecraft. */
static lin::Matrix3x3f const gyr_to_body(
#if defined(PAN_LEADER)
  {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
  }
#elif defined(PAN_FOLLOWER)
  {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
  }
#else
static_assert(false, "Must define PAN_LEADER or PAN_FOLLOWER");
#endif
);

static lin::Matrix3x3f const mag1_to_body(
#if defined(PAN_LEADER)
  {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
  }
#elif defined(PAN_FOLLOWER)
  {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
  }
#else
static_assert(false, "Must define PAN_LEADER or PAN_FOLLOWER");
#endif
);

static lin::Matrix3x3f const mag2_to_body(
#if defined(PAN_LEADER)
  {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
  }
#elif defined(PAN_FOLLOWER)
  {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
  }
#else
static_assert(false, "Must define PAN_LEADER or PAN_FOLLOWER");
#endif
);
}  // namespace imu
}  // namespace adcs

#endif
