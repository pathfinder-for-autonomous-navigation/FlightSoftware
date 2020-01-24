//
// src/adcs/havt_devices.hpp
// FlightSoftware
//
// Contributors:
//   Shihao Cao     sfc72@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_HAVT_DEVICES_HPP_
#define SRC_ADCS_HAVT_DEVICES_HPP_

namespace adcs {
namespace havt {

/** @enum Index
 *  Enumerates the bitset address of the devices in the havt table
 *  The first value corresponds to bitset[0]  */
enum Index : unsigned char {
  IMU_GYR,
  IMU_MAG1,
  IMU_MAG2,
  MTR1,
  MTR2,
  MTR3,
  RWA_POT,
  RWA_WHEEL1,
  RWA_WHEEL2,
  RWA_WHEEL3,
  RWA_ADC1,
  RWA_ADC2,
  RWA_ADC3,
  SSA_ADC1,
  SSA_ADC2,
  SSA_ADC3,
  SSA_ADC4,
  SSA_ADC5,
  //GYRO_HEATER, <- uncomment when device is added
  _LENGTH
};
}  // namespace havt
}  // namespace adcs

#endif