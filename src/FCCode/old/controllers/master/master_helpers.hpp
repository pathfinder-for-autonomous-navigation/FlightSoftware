#ifndef MASTER_HELPERS_HPP_
#define MASTER_HELPERS_HPP_

#include "../../comms/uplink_struct.hpp"
#include "../../state/EEPROMAddresses.hpp"
#include "../../state/state_holder.hpp"
#include "../constants.hpp"
#include <EEPROM.h>

namespace Master {
bool safe_hold_needed();
void stop_safe_hold();
void safe_hold();
void initialization_hold();

//! Master safe hold wait process working area
extern THD_WORKING_AREA(safe_hold_timer_workingArea, 2048);
//! Master safe hold wait process
extern THD_FUNCTION(safe_hold_timer, arg);
//! Master safe hold wait process thread pointer
extern thread_t *safe_hold_timer_thread;

bool standby_needed();

extern virtual_timer_t docking_timer;
void stop_docking_mode(void *args);

void apply_uplink_data();
void apply_uplink_commands();
void apply_uplink_resets();
// Helpers for uplink application
void apply_uplink_constants(const Comms::Uplink &uplink);
void apply_uplink_fc_hat(const Comms::Uplink &uplink);
void apply_uplink_adcs_hat(const Comms::Uplink &uplink);
} // namespace Master

#endif