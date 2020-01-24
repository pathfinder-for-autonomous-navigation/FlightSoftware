//
// src/adcs/imu_calibration.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

/* 
 * Just a quick note to whoever ends up performing the sensor calibration. The
 * general structure of the algorithm is just a loose structure that you may or
 * may not follow.
 * 
 * Essentially what I was going here for was find a matrix (rotation and
 * relative scaling) relationship as a function of temperature and a separate
 * offset relation as a function of temperature. This seems to be one of the
 * more straightforward ways of going about it.
 * 
 * Also, yes, the rotaton and scaling could be a single 4x4 matrix
 * multiplication if that relationship is easier to determine.
 * 
 */

#include "imu_calibration.hpp"

namespace adcs {
namespace imu {

// Commented out to avoid -Werror=unused-variable

// #define OFFSETS_LEN 2
// static lin::Vector3f const offsets[OFFSETS_LEN] = {
// #if defined(ADCS_LEADER)
//   lin::Vector3f({0.0f, 0.0f, 0.0f}),
//   lin::Vector3f({0.0f, 0.0f, 0.0f})
// #elif defined(ADCS_FOLLOWER)
//   lin::Vector3f({0.0f, 0.0f, 0.0f}),
//   lin::Vector3f({0.0f, 0.0f, 0.0f})
// #else
// static_assert(false, "Must define ADCS_LEADER or ADCS_FOLLOWER");
// #endif
// };

static void get_offset(lin::Vector3f &offset, float temp) {
  offset = {0.0f, 0.0f, 0.0f};
}

// #define TRANSFORMATIONS_LEN 2
// static lin::Matrix3x3f const transformations[TRANSFORMATIONS_LEN] = {
// #if defined(ADCS_LEADER)
//     lin::Matrix3x3f({
//     1.0f, 0.0f, 0.0f,
//     0.0f, 1.0f, 0.0f,
//     0.0f, 0.0f, 1.0f
//   }),
//   lin::Matrix3x3f({
//     1.0f, 0.0f, 0.0f,
//     0.0f, 1.0f, 0.0f,
//     0.0f, 0.0f, 1.0f
//   })
// #elif defined(ADCS_FOLLOWER)
//   lin::Matrix3x3f({
//     1.0f, 0.0f, 0.0f,
//     0.0f, 1.0f, 0.0f,
//     0.0f, 0.0f, 1.0f
//   }),
//   lin::Matrix3x3f({
//     1.0f, 0.0f, 0.0f,
//     0.0f, 1.0f, 0.0f,
//     0.0f, 0.0f, 1.0f
//   })
// #else
// static_assert(false, "Must define ADCS_LEADER or ADCS_FOLLOWER");
// #endif
// };

static void get_transformation(lin::Matrix3x3f &transformation, float temp) {
  transformation = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
  };
}

void calibrate(lin::Vector3f &omega, float temp) {
  lin::Matrix3x3f transformation;
  lin::Vector3f offset;
  get_transformation(transformation, temp);
  get_offset(offset, temp);
  omega = transformation * omega + offset;
}
}  // namespace imu
}  // namespace adcs
