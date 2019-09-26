//
// include/state_constants.hpp
// ADCS
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef PAN_ADCS_INCLUDE_STATE_CONSTANTS_HPP_
#define PAN_ADCS_INCLUDE_STATE_CONSTANTS_HPP_

#include <i2c_t3.h>
#include <AD5254.hpp>
#include <ADS1015.hpp>

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

/** \enum MTRMode
 *  Outlines all modes of the magnetic torque rods. */
enum MTRMode : unsigned char {
  /** Disables the magnetic torque rods. */
  MTR_DISABLED = 0,
  /** Places the magnetic torque rods under normal operation. */
  MTR_ENABLED = 1
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

/** Maximum temperature the gyroscope can be controlled to. */
static float const max_control_temp = 85.0f;
/** Minimum temperature the gyroscope can be controlled to. */
static float const min_control_temp = -40.0f;

/** Maximum temperature read from the gyroscope. */
static float const max_read_temp = 25.0f + 128.0f;
/** Minimum temperature read from the gyroscope. */
static float const min_read_temp = 25.0f - 128.0f;
/** Maximum angular rate read from the gyroscope. */
static float const max_omega = 125.0f * 0.03490658504f; // 2 * pi / 180
/** Minimum angular rate read from the gyroscope. */
static float const min_omega = -max_omega;

/** Maximum magnetic field reading from the magnetometers. This is 16 gauss. */
static float const max_mag = 0.0016f;
/** Minimum magnetic field readings from the magnetometers. */
static float const min_mag = -max_mag;

}  // namespace imu

namespace mtr {

/** Forward PWM pin for the zeroth magnetorquer. */
static unsigned int const mtr0_f_pin = 7;
/** Backward PWM pin for the zeroth magnetorquer. */
static unsigned int const mtr0_r_pin = 8;
/** Forward PWM pin for the first magnetorquer. */
static unsigned int const mtr1_f_pin = 35;
/** Backward PWM pin for the first magnetorquer. */
static unsigned int const mtr1_r_pin = 36;
/** Forward PWM pin for the second magnetorquer. */
static unsigned int const mtr2_f_pin = 2;
/** Backward PWM pin for the second magnetorquer. */
static unsigned int const mtr2_r_pin = 14;

/** Maximum moment available from the magnetic torque rods. */
static float const max_moment = 0.113337f / 2.0f; // Assumes a single mtr not two
/** Minimum moment available from the magnetic toque rods. */
static float const min_moment = -max_moment;

}  // namespace mtr

namespace rwa {

/** Wire for the potentiometer. */
static i2c_t3 *const pot_wire = &Wire1;
/** Address for the potentiomter. */
static unsigned char const pot_addr = AD5254::ADDR::A01;
/** Timeout in microseconds for the potentiometer. */
static unsigned long const pot_timeout = 10000;

/** Clockwise enable pin for the zeroth wheel. */
static unsigned int const wheel0_cw_pin = 26;
/** Counterclockwise enable pin for the zeroth wheel. */
static unsigned int const wheel0_ccw_pin = 39;
/** Speed servo pulse pin for the zeroth wheel. */
static unsigned int const wheel0_speed_pin = 23;
/** Clockwise enable pin for the first wheel. */
static unsigned int const wheel1_cw_pin = 10;
/** Counterclockwise enable pin for the first wheel. */
static unsigned int const wheel1_ccw_pin = 25;
/** Speed servo pulse pin for the first wheel. */
static unsigned int const wheel1_speed_pin = 22;
/** Clockwise enable pin for the second wheel. */
static unsigned int const wheel2_cw_pin = 9;
/** Counterclockwise enable pin for the second wheel. */
static unsigned int const wheel2_ccw_pin = 24;
/** Speed servo pulse pin for the second wheel. */
static unsigned int const wheel2_speed_pin = 21;

/** Wire for the zeroth wheel's ADC. */
static i2c_t3 *const adc0_wire = &Wire1;
/** Address for the zeroth wheel's ADC. */
static unsigned int const adc0_addr = ADS1015::ADDR::GND;
/** Alert pin for the zeroth wheel's ADC. */
static unsigned int const adc0_alrt = 11;
/** Wire for the first wheel's ADC. */
static i2c_t3 *const adc1_wire = &Wire1;
/** Address for the first wheel's ADC. */
static unsigned int const adc1_addr = ADS1015::ADDR::VDD;
/** Alert pin for the first wheel's ADC. */
static unsigned int const adc1_alrt = 12;
/** Wire for the second wheel's ADC. */
static i2c_t3 *const adc2_wire = &Wire1;
/** Address for the second wheel's ADC. */
static unsigned int const adc2_addr = ADS1015::ADDR::SSCL;
/** Alert pin for the second wheel's ADC. */
static unsigned int const adc2_alrt = 16;
/** Timeout value in microseconds for the wheel's ADCs. */
static unsigned long const adcx_timeout = 10000;

/* Maximum torque available from the reaction wheels. Note the maximum ramp maps
 * to 310.1852 rad s^-2 and the moment of inertia is 1.35e-5 kg m^2. */

/** Moment of interia of a reaction wheel. */
static float const moment_of_inertia = 0.0000135f;
/** Maximum torque available from the reaction wheels. */
static float const max_torque = 310.1852f * moment_of_inertia;
/** Minimum torque available from the reaction wheels. */
static float const min_torque = -max_torque;
/** Maximum speed read from the reaction wheels. */
static float const max_speed_read = 1047.20f;
/** Minimum speed read from the reaction wheels. */
static float const min_speed_read = -max_speed_read;

/** Maximum speed read from the reaction wheels. */
static float const max_speed_command = 680.678f;
/** Minimum speed command to the reaction wheels. */
static float const min_speed_command = -max_speed_command;

/** Maximum angular momentum of a reaction wheel. */
static float const max_momentum = max_speed_command * moment_of_inertia;
/** Minimum angular momentum of a reaction wheel. */
static float const min_momentum = -max_momentum;

}  // namespace rwa

namespace ssa {

/** Wire for the face two ADC. */
static i2c_t3 *const adc2_wire = &Wire1;
/** Address for the zeroth ADC. */
static unsigned char const adc2_addr = ADS1015::ADDR::SSDA;
/** Alert pin for the zeroth ADC. */
static unsigned int const adc2_alrt = 13;
/** Wire for the face three ADC. */
static i2c_t3 *const adc3_wire = &Wire1;
/** Address for the face three ADC. */
static unsigned char const adc3_addr = ADS1015::ADDR::GND;
/** Alert pin for the face three ADC. */
static unsigned int const adc3_alrt = 15;
/** Wire for the face four ADC. */
static i2c_t3 *const adc4_wire = &Wire1;
/** Address for the face four ADC. */
static unsigned char const adc4_addr = ADS1015::ADDR::VDD;
/** Alert pin for the face four ADC. */
static unsigned int const adc4_alrt = 20;
/** Wire for the face five ADC. */
static i2c_t3 *const adc5_wire = &Wire1;
/** Address for the face five ADC. */
static unsigned char const adc5_addr = ADS1015::ADDR::SSDA;
/** Alert pin for the face five ADC. */
static unsigned int const adc5_alrt = 27;
/** Wire for the face six ADC. */
static i2c_t3 *const adc6_wire = &Wire1;
/** Address for the face six ADC. */
static unsigned char const adc6_addr = ADS1015::ADDR::SSCL;
/** Alert pin for the six five ADC. */
static unsigned int const adc6_alrt = 28;
/** Timeout in microseconds for the ADCs. */
static unsigned long const adcx_timeout = 10000;

/** Required number of sensors in view of the sun to calculate the sun vector
 *  via least squares. */
static unsigned int const sensor_count_thresh = 4;
/** Relative voltage threshold for a sun sensor to be considered in view of the
 *  sun. The actual voltage cutoff is the magnitude of the normal vector times
 *  this value. */
static float const sensor_voltage_thresh = 0.5f;

}  // namespace ssa

namespace umb {

/** Wire used to communicate with the umbilical board. */
static i2c_t3 *const wire = &Wire;

}  // namespace umb

#endif
