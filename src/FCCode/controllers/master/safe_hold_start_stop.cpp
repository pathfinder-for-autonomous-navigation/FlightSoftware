#include "hold_functions.hpp"

using State::Master::master_state_lock;

void HoldFunctions::stop_safe_hold() {
    chThdTerminate(RTOSTasks::safe_hold_timer_thread);
    RTOSTasks::stop_safehold();
}

void HoldFunctions::safe_hold(unsigned short int reason) {
    // TODO write reason to state

    debug_println("Entering safe hold mode...");
    rwMtxWLock(&master_state_lock);
        State::Master::master_state = State::Master::MasterState::SAFE_HOLD;
        State::Master::pan_state = State::Master::PANState::MASTER_SAFEHOLD;
    rwMtxWUnlock(&master_state_lock);

    rwMtxWLock(&State::ADCS::adcs_state_lock);
        State::ADCS::adcs_state = State::ADCS::ZERO_TORQUE;
    rwMtxWUnlock(&State::ADCS::adcs_state_lock);

    rwMtxWLock(&State::Propulsion::propulsion_state_lock);
        State::ADCS::adcs_state = State::ADCS::ZERO_TORQUE;
    rwMtxWUnlock(&State::Propulsion::propulsion_state_lock);

    chMtxLock(&eeprom_lock);
        EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_FLAG, true);
    chMtxUnlock(&eeprom_lock);

    // Start safe hold timer
    RTOSTasks::safe_hold_timer_thread = chThdCreateStatic(&RTOSTasks::safe_hold_timer_workingArea, 
                                                    sizeof(RTOSTasks::safe_hold_timer_workingArea), 
                                                    RTOSTasks::master_thread_priority, 
                                                    RTOSTasks::safe_hold_timer, NULL);
}