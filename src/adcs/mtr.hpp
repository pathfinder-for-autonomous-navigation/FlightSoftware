//
// src/adcs/mtr.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_MTR_HPP_
#define SRC_ADCS_MTR_HPP_

#include "dev/Magnetorquer.hpp"

#include <lin.hpp>

namespace adcs {
namespace mtr {

/** MTR device array. */
extern dev::Magnetorquer mtrs[3];

/** @fn init
 *  Initiates the MTRs with a PWM of zero along each axis. */
void setup();

/** @fn actuate
 *  Actuates the MTRs according to the mode, commanded vector, and magnetic
 *  moment limit. Note that if the mode is not enabled, the magnetic torque rods
 *  will not actuate. */
void actuate(unsigned char mtr_mode, lin::Vector3f mtr_cmd, float mtr_lim);

}  // namespace mtr
}  // namespace adcs

#endif
