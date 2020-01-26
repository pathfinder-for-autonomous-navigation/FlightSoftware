//
// src/adcs/utl/logging.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_UTL_LOGGING_HPP_
#define SRC_ADCS_UTL_LOGGING_HPP_

#include <Arduino.h>

#ifndef LOG_SERIAL
#define LOG_SERIAL Serial
#endif

#if LOG_LEVEL > 0
#define LOG_ERROR(x) x
#else
#define LOG_ERROR(x)
#endif

#define LOG_ERROR_print(x)    LOG_ERROR(LOG_SERIAL.print(x);)
#define LOG_ERROR_printF(x)   LOG_ERROR(LOG_SERIAL.print(F(x));)
#define LOG_ERROR_println(x)  LOG_ERROR(LOG_SERIAL.println(x);)
#define LOG_ERROR_printlnF(x) LOG_ERROR(LOG_SERIAL.println(F(x));)
#define LOG_ERROR_header      LOG_WARN_printF("[log@adcs ERROR] $ ")

#if LOG_LEVEL > 1
#define LOG_WARN(x) x
#else
#define LOG_WARN(x)
#endif

#define LOG_WARN_print(x)    LOG_WARN(LOG_SERIAL.print(x);)
#define LOG_WARN_printF(x)   LOG_WARN(LOG_SERIAL.print(F(x));)
#define LOG_WARN_println(x)  LOG_WARN(LOG_SERIAL.println(x);)
#define LOG_WARN_printlnF(x) LOG_WARN(LOG_SERIAL.println(F(x));)
#define LOG_WARN_header      LOG_WARN_printF("[log@adcs WARN ] $ ")

#if LOG_LEVEL > 2
#define LOG_INFO(x) x
#else
#define LOG_INFO(x)
#endif

#define LOG_INFO_print(x)    LOG_INFO(LOG_SERIAL.print(x);)
#define LOG_INFO_printF(x)   LOG_INFO(LOG_SERIAL.print(F(x));)
#define LOG_INFO_println(x)  LOG_INFO(LOG_SERIAL.println(x);)
#define LOG_INFO_printlnF(x) LOG_INFO(LOG_SERIAL.println(F(x));)
#define LOG_INFO_header      LOG_WARN_printF("[log@adcs INFO ] $ ")

#if LOG_LEVEL > 3
#define LOG_TRACE(x) x
#else
#define LOG_TRACE(x)
#endif

#define LOG_TRACE_print(x)    LOG_TRACE(LOG_SERIAL.print(x);)
#define LOG_TRACE_printF(x)   LOG_TRACE(LOG_SERIAL.print(F(x));)
#define LOG_TRACE_println(x)  LOG_TRACE(LOG_SERIAL.println(x);)
#define LOG_TRACE_printlnF(x) LOG_TRACE(LOG_SERIAL.println(F(x));)
#define LOG_TRACE_header      LOG_WARN_printF("[log@adcs TRACE] $ ")

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
