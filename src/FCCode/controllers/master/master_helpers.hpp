#ifndef MASTER_HELPERS_HPP_
#define MASTER_HELPERS_HPP_

#include "../../state/state_holder.hpp"
#include <EEPROM.h>
#include "../../state/EEPROMAddresses.hpp"
#include "../constants.hpp"

namespace Master {
    unsigned short int safe_hold_needed();
    void stop_safe_hold();
    void safe_hold(unsigned short int reason);
    void initialization_hold(unsigned short int reason);
    
    extern virtual_timer_t docking_timer;
    void stop_docking_mode(void* args);
}

#endif