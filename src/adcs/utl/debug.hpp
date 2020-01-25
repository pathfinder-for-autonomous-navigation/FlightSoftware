//
// src/adcs/utl/debug.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_UTL_DEBUG_HPP_
#define SRC_ADCS_UTL_DEBUG_HPP_

#include <Arduino.h>

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

#ifdef DEBUG
#define DEBUG_HELPER(x) x
#else 
#define DEBUG_HELPER(x)
#endif

/** Initializes the debug serial port if the program was compiled with the macro
 *  DEBUG. */
#define DEBUG_init(baud) DEBUG_HELPER(DEBUG_SERIAL.begin(baud);)
/** Prints output to the main serial port if the program was compiled with the
 *  macro DEBUG. No new line charactar is included. */
#define DEBUG_print(x) DEBUG_HELPER(DEBUG_SERIAL.print(x);)
/** Flash, string-only version of DEBUG_print. It's similarly only enabled if
 *  the macro DEBUG is defined. */
#define DEBUG_printF(x) DEBUG_HELPER(DEBUG_SERIAL.print(F(x));)
/** Prints a new line charactar to the main serial port if the program was
 *  compiled with the macro DEBUG. */
#define DEBUG_println(x) DEBUG_HELPER(DEBUG_SERIAL.println(x);)
/** Flash, string-only version of DEBUG_println. It's similarly only enabled if
 *  the macro DEBUG is defined. */
#define DEBUG_printlnF(x) DEBUG_HELPER(DEBUG_SERIAL.println(F(x));)

#endif
