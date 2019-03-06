#include "master_helpers.hpp"

using State::Master::master_state_lock;

void RTOSTasks::stop_safehold() {
    rwMtxWLock(&State::Master::master_state_lock);
        State::Master::master_state = State::Master::MasterState::DETUMBLE;
        State::Master::pan_state = State::Master::PANState::MASTER_DETUMBLE;
        State::Master::autoexited_safe_hold = false; // Here we set autoexit to false, since this function
                                                     // could be called by an uplink packet
    rwMtxWUnlock(&State::Master::master_state_lock);

    chMtxLock(&eeprom_lock);
        EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_FLAG, false);
        EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_TIMER, (unsigned int) 0);
    chMtxUnlock(&eeprom_lock);

    rwMtxWLock(&State::Propulsion::propulsion_state_lock);
        State::Propulsion::propulsion_state = State::Propulsion::PropulsionState::IDLE;
    rwMtxWUnlock(&State::Propulsion::propulsion_state_lock);

    debug_println("Safe hold completed!");
}

void Master::stop_safe_hold() {
    chThdTerminate(RTOSTasks::safe_hold_timer_thread);
    RTOSTasks::stop_safehold();
}

void Master::safe_hold(unsigned short int reason) {
    debug_println("Entering safe hold mode...");
    rwMtxWLock(&master_state_lock);
        State::Master::master_state = State::Master::MasterState::SAFE_HOLD;
        State::Master::pan_state = State::Master::PANState::MASTER_SAFEHOLD;
    rwMtxWUnlock(&master_state_lock);

    // Disable ADCS
    rwMtxWLock(&State::ADCS::adcs_state_lock);
        State::ADCS::adcs_state = State::ADCS::ZERO_TORQUE;
    rwMtxWUnlock(&State::ADCS::adcs_state_lock);

    // Disable propulsion firings
    rwMtxWLock(&State::Propulsion::propulsion_state_lock);
        State::Propulsion::propulsion_state = State::Propulsion::PropulsionState::DISABLED;
    rwMtxWUnlock(&State::Propulsion::propulsion_state_lock);

    // Write to EEPROM
    chMtxLock(&eeprom_lock);
        EEPROM.put(EEPROM_ADDRESSES::FINAL_STATE_FLAG, (unsigned char) 0);
        EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_FLAG, true);
    chMtxUnlock(&eeprom_lock);

    // Start safe hold timer
    // TODO DON'T start timer if safe hold is due to mechanical ADCS failures. This will only
    // worsen ADCS problems.
    RTOSTasks::safe_hold_timer_thread = chThdCreateStatic(&RTOSTasks::safe_hold_timer_workingArea, 
                                                    sizeof(RTOSTasks::safe_hold_timer_workingArea), 
                                                    RTOSTasks::master_thread_priority, 
                                                    RTOSTasks::safe_hold_timer, NULL);
}