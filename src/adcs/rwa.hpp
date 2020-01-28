//
// src/adcs/rwa.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_RWA_HPP_
#define SRC_ADCS_RWA_HPP_

#include "dev/AD5254.hpp"
#include "dev/ADS1015.hpp"
#include "dev/MaxonEC45.hpp"

#include <lin.hpp>

namespace adcs {
namespace rwa {

/** Potentiometer used to control the analog ramp inputs. **/
extern dev::AD5254 potentiometer;

/** Reaction wheel motor drivers. **/
extern dev::MaxonEC45 wheels[3];

/** Analog to digital converters to read data from the motor drivers. **/
extern dev::ADS1015 adcs[3];

/** Wheel speed read by the controller. **/
extern lin::Vector3f speed_rd;

/** Analog ramp value read by the controller. **/
extern lin::Vector3f ramp_rd;

/** \fn setup
 *  Initializes all components involved in the reaction wheel system. **/
void setup();

/** \fn update_sensors
 *  \param[in] speed_flt
 *  \param[in] ramp_flt
 *   **/
void update_sensors(float speed_flt, float ramp_flt);

/** \fn control
 *  \param[in] rwa_mode
 *  \param[in] rwa_cmd
 *   */
void actuate(unsigned char rwa_mode, lin::Vector3f rwa_cmd);

}  // namespace rwa
}  // namespace adcs

#endif
