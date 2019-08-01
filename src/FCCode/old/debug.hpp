/** @file debug.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains declarations of functions that facilitate printing for
 * debugging.
 */

#include <ChRt.h>
#include "debug_console.hpp"

#ifndef DEBUG_HPP_
#define DEBUG_HPP_

#define DEBUG_ENABLED true

/**
 * @brief Initializes the EEPROM in a particular way in order to test the
 * spacecraft code with different initial conditions.
 *
 * See the implementation for more details.
 */
void debug_eeprom_initialization();

extern debug_console dbg;

/** @brief Working area for main debugger thread process. */
extern THD_WORKING_AREA(debug_workingArea, 4096);

/** @brief Main debugger thread process.
 * This process can be used to produce arbitrary debugging data. For example, we
 * can print stack sizes in this process--it wouldn't make sense to try to fit
 * that kind of data in any other process, so we put it in this process instead.
 * */
extern THD_FUNCTION(debug_function, arg);

#endif