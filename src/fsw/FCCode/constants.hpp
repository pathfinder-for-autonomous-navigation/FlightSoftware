#ifndef CONSTANTS_HPP_
#define CONSTANTS_HPP_

namespace PAN {
    // Environment-based initializations of the control loop time.
    // control_cycle_time is the value actually used for timing. The
    // other constants are just informational.
    #ifdef FUNCTIONAL_TEST
        constexpr uint32_t control_cycle_time_ms = 170;
        constexpr uint32_t control_cycle_time_us = 170000;
        #ifdef DESKTOP
            constexpr uint32_t control_cycle_time = 170000000;
        #else
            constexpr uint32_t control_cycle_time = 170000; 
        #endif
    #elif defined(FLIGHT) || defined(UNIT_TEST)
        constexpr uint32_t control_cycle_time_ms = 120;
        constexpr uint32_t control_cycle_time_us = 120000;
        constexpr uint32_t control_cycle_time = 120000;
    #endif

    constexpr uint32_t one_day_ccno = 24 * 60 * 60 * 1000 / control_cycle_time_ms;

    constexpr uint16_t packet_size = 70;
    constexpr uint16_t packet_size_bits = packet_size * 8;
} // namespace PAN

#endif
