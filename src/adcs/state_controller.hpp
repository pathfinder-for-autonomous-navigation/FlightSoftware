//
// src/adcs/state_controller.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_STATE_CONTROLLER_HPP_
#define SRC_ADCS_STATE_CONTROLLER_HPP_

#include <i2c_t3.h>

namespace adcs {
namespace umb {

/** Wire used to communicate with the umbilical board. */
static i2c_t3 *const wire = &Wire;

/** \fn on_i2c_recieve
 *  i2c message recieved callback to handle data pushes to the state struct. */
void on_i2c_recieve(unsigned int bytes);

/** \fn on_i2c_request
 *  i2c message requested callback to handl data pulls from the state struct. */
void on_i2c_request();

}  // namespace umb
}  // namespace adcs

#endif
