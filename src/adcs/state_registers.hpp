//
// src/adcs/state_registers.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_STATE_REGISTERS_HPP_
#define SRC_ADCS_STATE_REGISTERS_HPP_

namespace adcs {

/** @enum Register
 *  Enumerates the 'addresses' of the registers defined in the Registers struct.
 *  The below value should be the first and  */
enum Register : unsigned char {
  WHO_AM_I,
  ENDIANNESS,
  ADCS_MODE,
  READ_POINTER,
  RWA_MODE,
  RWA_COMMAND,
  RWA_COMMAND_FLAG,
  RWA_SPEED_FILTER,
  RWA_RAMP_FILTER,
  RWA_SPEED_RD,
  RWA_RAMP_READ,
  MTR_MODE,
  MTR_COMMAND,
  MTR_LIMIT,
  MTR_COMMAND_FLAG,
  SSA_MODE,
  SSA_SUN_VECTOR,
  SSA_VOLTAGE_FILTER,
  SSA_VOLTAGE_READ,
  SSA_VOLTAGE_THRESHOLD,
  IMU_MODE,
  IMU_MAG_READ,
  IMU_GYR_READ,
  IMU_GYR_TEMP_READ,
  IMU_MAG_FILTER,
  IMU_GYR_FILTER,
  IMU_GYR_TEMP_FILTER,
  IMU_GYR_TEMP_KP,
  IMU_GYR_TEMP_KI,
  IMU_GYR_TEMP_KD,
  IMU_GYR_TEMP_DESIRED,
  HAVT_COMMAND,
  HAVT_READ
};
}  // namespace adcs

#endif
