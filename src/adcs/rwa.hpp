//
// include/rwa/rwa.hpp
// ADCS
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef PAN_ADCS_INCLUDE_RWA_HPP_
#define PAN_ADCS_INCLUDE_RWA_HPP_

#include <lin.hpp>
#include <AD5254.hpp>
#include <ADS1015.hpp>
#include <MaxonEC45.hpp>

/** \namespace rwa
 *  Contains all functionality responsible for reaction wheel actuation. **/
namespace rwa {

/** Potentiometer used to control the analog ramp inputs. **/
extern AD5254 potentiometer;

/** Reaction wheel motor drivers. **/
extern MaxonEC45 wheels[3];

/** Analog to digital converters to read data from the motor drivers. **/
extern ADS1015 adcs[3];

/** Wheel speed read by the controller. **/
extern lin::Vector3f speed_rd;

/** Analog ramp value read by the controller. **/
extern lin::Vector3f ramp_rd;

/** \fn setup
 *  Initializes all components involved in the reaction wheel system. **/
extern void setup();

/** \fn update_sensors
 *  \param[in] speed_flt
 *  \param[in] ramp_flt
 *   **/
extern void update_sensors(float speed_flt, float ramp_flt);

/** \fn control
 *  \param[in] rwa_mode
 *  \param[in] rwa_cmd
 *   */
extern void control(unsigned char rwa_mode, lin::Vector3f rwa_cmd);

}  // namespace rwa

#endif
