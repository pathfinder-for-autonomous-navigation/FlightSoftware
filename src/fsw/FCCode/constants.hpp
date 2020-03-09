#ifndef CONSTANTS_HPP_
#define CONSTANTS_HPP_

#include <common/constant_tracker.hpp>

namespace PAN {
    // Environment-based initializations of the control loop time.
    // control_cycle_time is the value actually used for timing. The
    // other constants are just informational.
    #ifdef FUNCTIONAL_TEST
        TRACKED_CONSTANT_C(unsigned int, control_cycle_time_ms, 170)
        TRACKED_CONSTANT_C(unsigned int, control_cycle_time_us, 170000)
        #ifdef DESKTOP
            TRACKED_CONSTANT_C(unsigned int, control_cycle_time, 170000000)
        #else
            TRACKED_CONSTANT_C(unsigned int, control_cycle_time, 170000)
        #endif
    #elif defined(FLIGHT) || defined(UNIT_TEST)
        TRACKED_CONSTANT_C(unsigned int, control_cycle_time_ms, 120)
        TRACKED_CONSTANT_C(unsigned int, control_cycle_time_us, 120000)
        TRACKED_CONSTANT_C(unsigned int, control_cycle_time, 120000)
    #endif

    TRACKED_CONSTANT_C(unsigned int, one_day_ccno, 24 * 60 * 60 * 1000 / control_cycle_time_ms)
}

#endif
