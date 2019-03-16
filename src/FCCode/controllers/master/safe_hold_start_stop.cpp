#include "master_helpers.hpp"

using State::Master::master_state_lock;

void RTOSTasks::stop_safehold() {
    State::write(State::Master::master_state, State::Master::MasterState::DETUMBLE, master_state_lock);
    State::write(State::Master::pan_state, State::Master::PANState::MASTER_DETUMBLE, master_state_lock);
    // Here we set autoexit to false, since this function could be called by an uplink packet
    State::write(State::Master::autoexited_safe_hold, false, State::Master::master_state_lock); 

    chMtxLock(&eeprom_lock);
        EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_FLAG, false);
        EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_TIMER, (unsigned int) 0);
    chMtxUnlock(&eeprom_lock);

    State::write(State::Propulsion::propulsion_state, State::Propulsion::PropulsionState::IDLE, State::Propulsion::propulsion_state_lock);
    debug_println("Safe hold completed!");
}

void Master::stop_safe_hold() {
    chThdTerminate(RTOSTasks::safe_hold_timer_thread);
    RTOSTasks::stop_safehold();
}

void Master::safe_hold() {
    debug_println("Entering safe hold mode...");
    State::write(State::Master::master_state, State::Master::MasterState::SAFE_HOLD, master_state_lock);
    State::write(State::Master::pan_state, State::Master::PANState::MASTER_SAFEHOLD, master_state_lock);

    // Disable ADCS
    State::write(State::ADCS::adcs_state, State::ADCS::ZERO_TORQUE, State::ADCS::adcs_state_lock);
    // Disable propulsion firings
    State::write(State::Propulsion::propulsion_state,
                 State::Propulsion::PropulsionState::DISABLED,
                 State::Propulsion::propulsion_state_lock);

    // Write to EEPROM
    chMtxLock(&eeprom_lock);
        EEPROM.put(EEPROM_ADDRESSES::FINAL_STATE_FLAG, (unsigned char) 0);
        EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_FLAG, true);
    chMtxUnlock(&eeprom_lock);

    // Start safe hold timer
    RTOSTasks::safe_hold_timer_thread = chThdCreateStatic(&RTOSTasks::safe_hold_timer_workingArea, 
                                                    sizeof(RTOSTasks::safe_hold_timer_workingArea), 
                                                    RTOSTasks::master_thread_priority, 
                                                    RTOSTasks::safe_hold_timer, NULL);
}