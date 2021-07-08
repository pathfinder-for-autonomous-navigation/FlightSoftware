#ifndef CONSTANTS_HPP_
#define CONSTANTS_HPP_

#include <common/constant_tracker.hpp>

namespace PAN {
    // Environment-based initializations of the control loop time.
    // control_cycle_time is the value actually used for timing. The
    // other constants are just informational.
    #ifdef FLIGHT
        TRACKED_CONSTANT_SC(unsigned int, control_cycle_time_ms, 170);
        TRACKED_CONSTANT_SC(unsigned int, control_cycle_time_us, control_cycle_time_ms * 1000);
        TRACKED_CONSTANT_SC(unsigned int, control_cycle_time_ns, control_cycle_time_us * 1000);
        TRACKED_CONSTANT_SC(unsigned int, control_cycle_time, control_cycle_time_us);
    #else
        TRACKED_CONSTANT_SC(unsigned int, control_cycle_time_ms, 170);
        TRACKED_CONSTANT_SC(unsigned int, control_cycle_time_us, control_cycle_time_ms * 1000);
        TRACKED_CONSTANT_SC(unsigned int, control_cycle_time_ns, control_cycle_time_us * 1000);
        #ifdef DESKTOP
            TRACKED_CONSTANT_SC(unsigned int, control_cycle_time, control_cycle_time_ns);
        #else
            TRACKED_CONSTANT_SC(unsigned int, control_cycle_time, control_cycle_time_us);
        #endif
    #endif

    # ifdef QFH_SPEEDUP
        TRACKED_CONSTANT_SC(unsigned int, one_day_ccno, 1400);
    # endif

    #ifdef SPEEDUP
        TRACKED_CONSTANT_SC(unsigned int, one_day_ccno, 10 * 1000 / control_cycle_time_ms);
    #else
        TRACKED_CONSTANT_SC(unsigned int, one_day_ccno, 24 * 60 * 60 * 1000 / control_cycle_time_ms);
    #endif
}

#endif
