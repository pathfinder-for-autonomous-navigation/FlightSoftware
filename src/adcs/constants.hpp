//
// src/adcs/constants.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

// TODO
// Calibrate gyroscope min and max temperature

#ifndef SRC_ADCS_ADCS_CONSTANTS_HPP_
#define SRC_ADCS_ADCS_CONSTANTS_HPP_

#include <common/constant_tracker.hpp>

namespace adcs {

/** \enum Endianness
 *  Sets the endianness of the I2C transmissions. **/
enum Endianness : unsigned char {
  /** Sets the I2C messages to little-endian interpretation. **/
  LITTLE = 0,
  /** Sets the I2C messages to big-endian interpretation. **/
  BIG = 1
};

/** \enum ADCSMode
 *  Enumeration of ADCS modes. **/
enum ADCSMode : unsigned char {
  /** Only passive sensor measurements are taken. **/
  ADCS_PASSIVE = 0,
  /** All ADCS peripherials are enabled. **/
  ADCS_ACTIVE = 1
};

/** \enum IMUMAGMode
 *  Outlines all modes of the IMU magnetometers. */
enum IMUMAGMode : unsigned char {
  /** Continue normal operation - i.e. no calibration. */
  IMU_MAG_NORMAL = 0,
  /** Calibrate the magnetometer. */
  IMU_MAG_CALIBRATE = 1
};

/** \enum MTRMode
 *  Outlines all modes of the magnetic torque rods. */
enum MTRMode : unsigned char {
  /** Disables the magnetic torque rods. */
  MTR_DISABLED = 0,
  /** Places the magnetic torque rods under normal operation. */
  MTR_ENABLED = 1
};

/** \enum RWAMode
 *  Outlines all modes of the reaction wheels. **/
enum RWAMode : unsigned char {
  /** Disables the reaction wheels. */
  RWA_DISABLED = 0,
  /** Speed controlled mode. **/
  RWA_SPEED_CTRL = 1,
  /** Acceleration controlled mode. **/
  RWA_ACCEL_CTRL = 2
};

/** \enum SSAMode
 *  Outlines all modes of the sun sensors. **/
enum SSAMode : unsigned char {
  /** Conversion may have failed and the sun vector is not realiable. **/
  SSA_FAILURE = 0,
  /** Conversion to sun vector in progress. **/
  SSA_IN_PROGRESS = 1,
  /** Succesful conversion has been completed. **/
  SSA_COMPLETE = 2
};

/** \enum CMDFlag
 *  Outlines all possible actuation command modes. */
enum CMDFlag : unsigned char {
  /** No new actuation command is present. */
  OUTDATED = 0,
  /** A new actuation command is present. */
  UPDATED = 1
};

namespace imu {

/** Highest temperature in degrees celcius the gyroscope's equilibrium
 *  temperature can be set to. */
TRACKED_CONSTANT_SC(float, max_eq_temp, 85.0f);
/** Lowest termperature in degrees celcius the gyroscope's equilibrium
 * temperature can be set to. */
TRACKED_CONSTANT_SC(float, min_eq_temp, -40.0f);
/** Maximum temperature reading in degress celcius that can be output by the
 *  gyroscope. */
#if defined(PAN_LEADER)
TRACKED_CONSTANT_SC(float, max_rd_temp, 25.0f + 128.0f); // TODO : Calibrate
#elif defined(PAN_FOLLOWER)
TRACKED_CONSTANT_SC(float, max_rd_temp, 25.0f + 128.0f); // TODO : Calibrate
#else
static_assert(false, "Must define PAN_LEADER or PAN_FOLLOWER");
#endif
/** Minimum temperature reading in degrees celcius that can be output by the
 *  gyroscope. */
#if defined(PAN_LEADER)
TRACKED_CONSTANT_SC(float, min_rd_temp, 25.0f - 128.0f); // TODO : Calibrate
#elif defined(PAN_FOLLOWER)
TRACKED_CONSTANT_SC(float, min_rd_temp, 25.0f - 128.0f); // TODO : Calibrate
#else
static_assert(false, "Must define PAN_LEADER or PAN_FOLLOWER");
#endif
/** Maximum angular rate in radians per second that can be read from the
 *  gyroscope. */
TRACKED_CONSTANT_SC(float, max_rd_omega, 125.0f * 0.01745329251f); // pi / 180
/** Minimum angular rate in radians per second that can be read from the
 *  gyroscope. */
TRACKED_CONSTANT_SC(float, min_rd_omega, -max_rd_omega);

/** Maximum magnetic field reading in Tesla that can be read from the first
 *  magnetometer. */
TRACKED_CONSTANT_SC(float, max_mag1_rd_mag, 0.0049152f);
/** Minimum magnetic field reading in Tesla that can be read from the first
 *  magnetometer. */
TRACKED_CONSTANT_SC(float, min_mag1_rd_mag, -max_mag1_rd_mag);

/** Maximum magnetic field reading in Tesla that can be read from the second
 *  magnetometer. */
TRACKED_CONSTANT_SC(float, max_mag2_rd_mag, 0.0016f);
/** Minimum magnetic field reading in Tesla that can be read from the second
 *  magnetometer. */
TRACKED_CONSTANT_SC(float, min_mag2_rd_mag, -max_mag2_rd_mag);

}  // namespace imu

namespace mtr {

/** Maximum moment available from the magnetic torque rods. */
TRACKED_CONSTANT_SC(float, max_moment, 0.113337f / 2.0f); // Assumes one mtr not two
/** Minimum moment available from the magnetic toque rods. */
TRACKED_CONSTANT_SC(float, min_moment, -max_moment);

}  // namespace mtr

namespace rwa {

/* Maximum torque available from the reaction wheels. Note the maximum ramp maps
 * to 310.1852 rad s^-2 and the moment of inertia is 1.35e-5 kg m^2. */

/** Moment of interia of a reaction wheel. */
TRACKED_CONSTANT_SC(float, moment_of_inertia, 0.0000135f);
/** Maximum torque available from the reaction wheels. */
TRACKED_CONSTANT_SC(float, max_torque, 310.1852f * moment_of_inertia);
/** Minimum torque available from the reaction wheels. */
TRACKED_CONSTANT_SC(float, min_torque, -max_torque);
/** Maximum ramp reading possible. */
TRACKED_CONSTANT_SC(float, max_ramp_rd, max_torque);
/** Minimum ramp reading possible. */
TRACKED_CONSTANT_SC(float, min_ramp_rd, 0);
/** Maximum speed read from the reaction wheels. */
TRACKED_CONSTANT_SC(float, max_speed_read, 1047.20f);
/** Minimum speed read from the reaction wheels. */
TRACKED_CONSTANT_SC(float, min_speed_read, -max_speed_read);

/** Maximum speed read from the reaction wheels. */
TRACKED_CONSTANT_SC(float, max_speed_command, 680.678f);
/** Minimum speed command to the reaction wheels. */
TRACKED_CONSTANT_SC(float, min_speed_command, -max_speed_command);

}  // namespace rwa

namespace ssa {

/** Minimum voltage that can be read from a sun sensor. */
TRACKED_CONSTANT_SC(float, min_voltage_rd, 0.0f);
/** Maximum voltage that can be read from a sun sensor. */
TRACKED_CONSTANT_SC(float, max_voltage_rd, 3.3f);
/** Minimum SSA algorithm voltage threshold. */
TRACKED_CONSTANT_SC(float, min_voltage_thresh, min_voltage_rd);
/** Maximum SSA algorithm voltage threshold. */
TRACKED_CONSTANT_SC(float, max_voltage_thresh, max_voltage_rd);
/** Number of sun sensors */
TRACKED_CONSTANT_SC(unsigned char, num_sun_sensors, 20);

}  // namespace ssa

namespace havt {

/** Maximum number of devices in the HAVT table. Leave as 32, not meant to be adjusted */
TRACKED_CONSTANT_SC(unsigned char, max_devices, 32);

} // namespace havt
}  // namespace adcs

#endif
