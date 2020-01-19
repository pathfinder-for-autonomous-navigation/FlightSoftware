//
// include/mtr/mtr.hpp
// ADCS
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef PAN_ADCS_INCLUDE_MTR_HPP_
#define PAN_ADCS_INCLUDE_MTR_HPP_

#include <Magnetorquer.hpp>
#include <lin.hpp>

/** @namespace mtr
 *  Contains all functionality responsible for MTR actuation. */
namespace mtr {

/** MTR device array. */
extern Magnetorquer mtrs[3];

/** @fn init
 *  Initiates the MTRs with a PWM of zero along each axis. */
extern void setup();

/** @fn actuate
 *  Actuates the MTRs according to the mode, commanded vector, and magnetic
 *  moment limit. Note that if the mode is not enabled, the magnetic torque rods
 *  will not actuate. */
extern void actuate(unsigned char mtr_mode, lin::Vector3f mtr_cmd, float mtr_lim);

}  // namespace mtr

#endif
