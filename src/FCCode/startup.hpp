/** @file startup.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains function definitions for the startup code.
 */

#ifndef STARTUP_HPP_
#define STARTUP_HPP_

//! Sets up satellite hardware.
void hardware_setup();
//! Sets up satellite system by checking initial conditions and starting up daughter processes that control the satellite.
void pan_system_setup();

#endif