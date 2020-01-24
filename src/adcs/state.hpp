//
// src/adcs/state.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

// TODO : Initialize the PID controller values for the gyroscope heater
// TODO : Complete HAVT array

#ifndef SRC_ADCS_STATE_HPP_
#define SRC_ADCS_STATE_HPP_

namespace adcs {

/** \struct ReactionWheelRegisters
 *  State data structure for the reaction wheel assembly. All commands and reads
 *  are assumed to be in the body frame of the spacecraft. It is assumed that
 *  three reaction wheels each parallel or antiparallel to one of the body frame
 *  axis. */
struct ReactionWheelRegisters {
  /** Specifies the reaction wheel's operating mode. */
  unsigned char mode;
  /** Reaction wheel actuation command in the body frame of the spacecraft. */
  float cmd[3];
  /** Flag for a new actuation command. */
  unsigned char cmd_flg;
  /** Specifies the exponential filter contant for the angular momentum read. */
  float momentum_flt;
  /** Specifies the exponential filter constant for the analog ramp read. */
  float ramp_flt;
  /** Current output value for an angular momentum read in the body frame of the
   *  spacecraft. */
  float momentum_rd[3];
  /** Current output value for an analog ramp read in the body frame of the
   *  spacecraft. */
  float ramp_rd[3];
};

/** \struct MagnetorquerRegisters
 *  State data structure for the magnetic torque rods. All commands are assumed
 *  to be in the body frame of the spacecraft with each MTR parallel or
 *  antiparallel tto one of the body frame axis. */
struct MagnetorquerRegisters {
  /** Specifies the magnetic torque rod's operating mode. */
  unsigned char mode;
  /** Magnetic torque rod actutation command Am^2 in the body frame of the
   *  spacecraft. */
  float cmd[3];
  /** Magnetic moment limiter along each access. */
  float moment_limit;
  /** Flag for a new actuation command. */
  unsigned char cmd_flg;
};

/** \struct SunSensorRegisters
 *  State data structure for the sun sensor assembly. All reads are in the body
 *  frame of the spacecraft. */
struct SunSensorRegisters {
  /** Specifies the current mode for the sun sensor assembly. */
  unsigned char mode;
  /** Sun vector resulting from the previous sun vector calculation. The sun
   *  vector is given in the body frame of the spacecraft. */
  float sun_vec_rd[3];
  /** Specifies the exponential filter constant for the sun sensor voltage
   *  readings. */
  float voltage_flt;
  /** Most recent individual sun sensor voltage readings. */
  float voltage_rd[20];
};

/** \struct IMURegisters
 *  State data structure for the IMU sensors. all reads are in the body frame of
 *  the spacecraft. */
struct IMURegisters {
  /** Specifies the current IMU mode - i.e. which magnetometer to talk to. */
  unsigned char mode;
  /** Current output for the magnetometer in units of Tesla in the body frame of
   *  the spacecraft. */
  float mag_rd[3];
  /** Current output for the gyroscope in units of rad/s in the body frame of
   *  the spacecraft. */
  float gyr_rd[3];
  /** Current output for the gyroscope temperature in units of Kelvin. */
  float gyr_temp_rd;
  /** Specifies the exponential filter constant for the magnetometer reads. */
  float mag_flt;
  /** Specifies the exponential filter constant for the gyroscope reads. */
  float gyr_flt;
  /** Specifies the exponential filter constant for the gyroscope temperature
   *  reads. */
  float gyr_temp_flt;
  /** Specifies the gyroscope temperature controller K_p term. */
  float gyr_temp_p;
  /** Specifies the gyroscope temperature controller K_i term. */
  float gyr_temp_i;
  /** Specifies the gyroscope temperature controller K_d term. */
  float gyr_temp_d;
  /** Specifies the desired temperature of the gyroscope. */
  float gyr_desired_temp;
};

/** \struct HAVTRegisters
 *  State data structure for the HAVT Tables.*/
struct HAVTRegisters {
  /** This table is updated with the most recent is_functional() for
   * each box aboard the ADCS Box. */
  unsigned int read_table;
  /** An intermediate data structure that will specify the requested reset() and disable()
   * for devices within the ADCS Box. */
  unsigned int cmd_table;
  /** This flag specifies if there is a new_flag, and is set to outdated
   * once the cmd_table has been applied. */
  unsigned char cmd_flg;
};

/** \struct Registers
 *  State data structure for the entire ADCS system. See comments for subsystem
 *  state structs for more information. */
struct Registers {
  /** Dummy who am I register for i2c initialization ping. */
  unsigned char const who_am_i;
  /** Specifies the i2c endianess setting the ADCS system will adhere to. */
  unsigned char endianess;
  /** Specifies the overall mode of the ADCS system. */
  unsigned char mode;
  /** Specifies the register location the next read will originate from. */
  unsigned char read_ptr;
  /** Reaction wheel state struct. */
  struct ReactionWheelRegisters rwa;
  /** Magnetorquer state struct. */
  struct MagnetorquerRegisters mtr;
  /** Sun sensor state struct. */
  struct SunSensorRegisters ssa;
  /** IMU state struct. */
  struct IMURegisters imu;
  /** Hardware availability table bit string. */
  struct HAVTRegisters havt;
};

/** Overall state struct for the ADCS system. This struct will be edited by both
 *  the i2c interupt service routines and the main thread processes. */
extern struct Registers volatile registers;

}  // namespace adcs

#endif
