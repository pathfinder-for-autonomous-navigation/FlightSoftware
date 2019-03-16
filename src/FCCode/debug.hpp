/** @file debug.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains declarations of functions that facilitate printing for
 * debugging.
 */

#include <chRt.h>

#ifndef DEBUG_HPP_
#define DEBUG_HPP_

/** @brief If set to true, console debugging is enabled. */
#define DEBUG true

/**
 * @brief Starts the debug console.
 */
void debug_begin();

/**
 * @brief Initializes the EEPROM in a particular way in order to test the spacecraft code with
 * different initial conditions. 
 * 
 * See the implementation for more details.
 */
void debug_eeprom_initialization();

/**
 * @brief Prints a formatted string and prepends the process name at the beginning of the string.
 * The use of a formatted string allows for the easy printing of arbitrary data.
 * @param format The format string specifying how data should be represented.
 * @param ... One or more arguments containing the data to be printed.
 */
void debug_printf(const char* format, ...);

/**
 * @brief Prints a formatted string without any process name.
 * @param format The format string specifying how data should be represented.
 * @param ... One or more arguments containing the data to be printed.
 */
void debug_printf_headless(const char* format, ...);

/**
 * @brief Prints a string and prepends the process name at the beginning of the string.
 * This allows for easy identification of which process a particular debugger message
 * came from.
 * @param str The string to be printed. 
 */
void debug_println(const char* str);

/**
 * @brief Prints a string without any process name.
 * @param str The string to be printed.
 */
void debug_println_headless(const char* str);

/**
 * @brief Prints a newline without any process name.
 */
void debug_println_headless(void);

void debug_blink_led();

/** @brief Prints an ASCII-based PAN logo. */
void print_pan_logo();
 
 /** @brief Working area for main debugger thread process. */
extern THD_WORKING_AREA(debug_workingArea, 4096);

/** @brief Main debugger thread process.
 * This process can be used to produce arbitrary debugging data. For example, we can print
 * stack sizes in this process--it wouldn't make sense to try to fit that kind of data in
 * any other process, so we put it in this process instead.
 * */
extern THD_FUNCTION(debug_function, arg);

#endif