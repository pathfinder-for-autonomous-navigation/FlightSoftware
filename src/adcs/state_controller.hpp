//
// include/state_controller.hpp
// ADCS
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef PAN_ADCS_INCLUDE_STATE_CONTROLLER_HPP_
#define PAN_ADCS_INCLUDE_STATE_CONTROLLER_HPP_

#include <i2c_t3.h>

namespace umb {

/** Wire used to communicate with the umbilical board. */
static i2c_t3 *const wire = &Wire;

/** \fn on_i2c_recieve
 *  i2c message recieved callback to handle data pushes to the state struct. */
extern void on_i2c_recieve(unsigned int bytes);

/** \fn on_i2c_request
 *  i2c message requested callback to handl data pulls from the state struct. */
extern void on_i2c_request();

}  // namespace umb

#endif
