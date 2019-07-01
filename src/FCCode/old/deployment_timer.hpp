/** @file deployment_timer.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains the deployment timer code's thread-related object definitions.
 */

#include <ChRt.h>

#ifndef DEPLOYMENT_TIMER_HPP_
#define DEPLOYMENT_TIMER_HPP_

#ifdef DEBUG_ENABLED
static constexpr unsigned int DEPLOYMENT_LENGTH = 5; // in seconds
#else
static constexpr unsigned int DEPLOYMENT_LENGTH = 30*60; // in seconds
#endif

//! Thread object for the deployment timer.
extern thread_t* deployment_timer_thread;
//! Thread queue for threads that are waiting for the deployment timer to finish.
extern threads_queue_t deployment_timer_waiting;
//! Working area for deployment timer thread.
extern THD_WORKING_AREA(deployment_timer_workingArea, 4096);
//! Function that defines the deployment timer thread.
extern THD_FUNCTION(deployment_timer_function, arg);

#endif