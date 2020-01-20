#ifndef CONSTANTS_HPP_
#define CONSTANTS_HPP_

namespace PAN {
    // Environment-based initializations of the control loop time.
    // control_cycle_time is the value actually used for timing. The
    // other constants are just informational.
    #ifdef FUNCTIONAL_TEST
        constexpr unsigned int control_cycle_time_ms = 170;
        constexpr unsigned int control_cycle_time_us = 170000;
        #ifdef DESKTOP
            constexpr unsigned int control_cycle_time = 170000000;
        #else
            constexpr unsigned int control_cycle_time = 170000; 
        #endif
    #elif defined(FLIGHT) || defined(UNIT_TEST)
        constexpr unsigned int control_cycle_time_ms = 120;
        constexpr unsigned int control_cycle_time_us = 120000;
        constexpr unsigned int control_cycle_time = 120000;
    #endif
}

#endif
