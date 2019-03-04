#ifndef HOLD_FUNCTIONS_HPP_
#define HOLD_FUNCTIONS_HPP_

// #include <utility>
// #include <Piksi/GPSTime.hpp>
// #include <tensor.hpp>
// #include <AttitudeEstimator.hpp>
// #include "../controllers.hpp"
// #include <Device.hpp>
// #include "../constants.hpp"
// #include "../../deployment_timer.hpp"
// #include "../../state/state_holder.hpp"
// #include "../../comms/apply_uplink.hpp"
// #include <AttitudeMath.hpp>

#include "../../state/state_holder.hpp"
#include <EEPROM.h>
#include "../../state/EEPROMAddresses.hpp"
#include "../constants.hpp"

namespace HoldFunctions {
    unsigned short int safe_hold_needed();
    void stop_safe_hold();
    void safe_hold(unsigned short int reason);
    void initialization_hold(unsigned short int reason);
}

#endif