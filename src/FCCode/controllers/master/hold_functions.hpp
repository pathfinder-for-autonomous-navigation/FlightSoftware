#ifndef HOLD_FUNCTIONS_HPP_
#define HOLD_FUNCTIONS_HPP_

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