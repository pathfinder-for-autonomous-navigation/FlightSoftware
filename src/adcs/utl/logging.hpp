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

// Enumerated logging levels
#define LOG_LEVEL_NONE  0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_INFO  3
#define LOG_LEVEL_TRACE 4

// Default the logging serial port to 'Serial'
#ifndef LOG_SERIAL
  #define LOG_SERIAL Serial
#endif

// Default the loging level to 'NONE'
#ifndef LOG_LEVEL
  #define LOG_LEVEL LOG_LEVEL_NONE
#endif

#if LOG_LEVEL >= LOG_LEVEL_ERROR
  #define LOG_init(x)  LOG_SERIAL.begin(x);
  #define LOG_ERROR(x) x
#else
  #define LOG_init(x)
  #define LOG_ERROR(x)
#endif

#define LOG_ERROR_print(x)    LOG_ERROR(LOG_SERIAL.print(x);)
#define LOG_ERROR_printF(x)   LOG_ERROR(LOG_SERIAL.print(F(x));)
#define LOG_ERROR_println(x)  LOG_ERROR(LOG_SERIAL.println(x);)
#define LOG_ERROR_printlnF(x) LOG_ERROR(LOG_SERIAL.println(F(x));)
#define LOG_ERROR_header      LOG_ERROR_printF("[log@adcs ERROR] $ ")

#if LOG_LEVEL >= LOG_LEVEL_WARN
  #define LOG_WARN(x) x
#else
  #define LOG_WARN(x)
#endif

#define LOG_WARN_print(x)    LOG_WARN(LOG_SERIAL.print(x);)
#define LOG_WARN_printF(x)   LOG_WARN(LOG_SERIAL.print(F(x));)
#define LOG_WARN_println(x)  LOG_WARN(LOG_SERIAL.println(x);)
#define LOG_WARN_printlnF(x) LOG_WARN(LOG_SERIAL.println(F(x));)
#define LOG_WARN_header      LOG_WARN_printF("[log@adcs WARN ] $ ")

#if LOG_LEVEL >= LOG_LEVEL_INFO
  #define LOG_INFO(x) x
#else
  #define LOG_INFO(x)
#endif

#define LOG_INFO_print(x)    LOG_INFO(LOG_SERIAL.print(x);)
#define LOG_INFO_printF(x)   LOG_INFO(LOG_SERIAL.print(F(x));)
#define LOG_INFO_println(x)  LOG_INFO(LOG_SERIAL.println(x);)
#define LOG_INFO_printlnF(x) LOG_INFO(LOG_SERIAL.println(F(x));)
#define LOG_INFO_header      LOG_INFO_printF("[log@adcs INFO ] $ ")

#if LOG_LEVEL >= LOG_LEVEL_TRACE
  #define LOG_TRACE(x) x
#else
  #define LOG_TRACE(x)
#endif

#define LOG_TRACE_print(x)    LOG_TRACE(LOG_SERIAL.print(x);)
#define LOG_TRACE_printF(x)   LOG_TRACE(LOG_SERIAL.print(F(x));)
#define LOG_TRACE_println(x)  LOG_TRACE(LOG_SERIAL.println(x);)
#define LOG_TRACE_printlnF(x) LOG_TRACE(LOG_SERIAL.println(F(x));)
#define LOG_TRACE_header      LOG_TRACE_printF("[log@adcs TRACE] $ ")

#endif
