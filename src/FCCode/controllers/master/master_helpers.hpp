#ifndef MASTER_HELPERS_HPP_
#define MASTER_HELPERS_HPP_

#include "../../state/state_holder.hpp"
#include <EEPROM.h>
#include "../../state/EEPROMAddresses.hpp"
#include "../constants.hpp"
#include "../../comms/uplink_struct.hpp"

namespace Master {
    bool safe_hold_needed();
    void stop_safe_hold();
    void safe_hold();
    void initialization_hold();

    bool standby_needed();
    
    extern virtual_timer_t docking_timer;
    void stop_docking_mode(void* args);

    void apply_uplink_data();
    void apply_uplink_commands();
    void apply_uplink_resets();
    // Helpers for uplink application
    void apply_uplink_constants(const Comms::Uplink& uplink);
    void apply_uplink_fc_hat(const Comms::Uplink& uplink);
    void apply_uplink_adcs_hat(const Comms::Uplink& uplink);
}

#endif