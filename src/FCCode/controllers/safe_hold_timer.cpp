/** @file deployment_timer.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains the deployment timer code, which is responsible for timing and notifying
 * all threads when deployment timing is complete.
 */

#include <EEPROM.h>
#include "../state/EEPROMAddresses.hpp"
#include "../state/state_holder.hpp"
#include "controllers.hpp"
#include "constants.hpp"
#include "../debug.hpp"

namespace RTOSTasks {
    thread_t* safe_hold_timer_thread;
    THD_WORKING_AREA(safe_hold_timer_workingArea, 2048);
}

void RTOSTasks::stop_safehold() {
    rwMtxWLock(&State::Master::master_state_lock);
        State::Master::master_state = State::Master::MasterState::NORMAL;
        if (State::Master::is_follower) State::Master::pan_state = State::Master::PANState::FOLLOWER; // TODO is this what we always want to do?
        else State::Master::pan_state = State::Master::PANState::STANDBY;
        State::Master::autoexited_safe_hold = false; // Here we set autoexit to false, since this function
                                                     // could be called by an uplink packet
    rwMtxWUnlock(&State::Master::master_state_lock);

    chMtxLock(&eeprom_lock);
        EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_FLAG, false);
        EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_TIMER_1, (unsigned int) 0);
    chMtxUnlock(&eeprom_lock);

    debug_println("Safe hold completed!");
}

//! Function that defines the safe hold timer thread.
void RTOSTasks::safe_hold_timer(void *arg) {
    chRegSetThreadName("SAFEHOLD TIMER");
    // Determine time remaining in safe hold
    chMtxLock(&eeprom_lock);
        unsigned char time_elapsed;
        EEPROM.get(EEPROM_ADDRESSES::SAFE_HOLD_TIMER_1, time_elapsed);
    chMtxUnlock(&eeprom_lock);

    // Start safe hold timer
    while(time_elapsed < Constants::Master::SAFE_HOLD_TIMEOUT) {
        // If no longer in safe hold, don't keep running down the timer!
        rwMtxRLock(&State::Master::master_state_lock);
            if (State::Master::master_state != State::Master::MasterState::SAFE_HOLD) {
                rwMtxRUnlock(&State::Master::master_state_lock);
                break;
            }
        rwMtxRUnlock(&State::Master::master_state_lock);

        chMtxLock(&eeprom_lock);
            EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_TIMER_1, time_elapsed);
        chMtxUnlock(&eeprom_lock);

        debug_printf("Time remaining until safe hold wait completed: %d\n", Constants::Master::SAFE_HOLD_TIMEOUT - time_elapsed);
        chThdSleepSeconds(1);
        time_elapsed++;
    }

    stop_safehold();
    rwMtxWLock(&State::Master::master_state_lock);
        State::Master::autoexited_safe_hold = true; // Here we set autoexit to true, since we can only get
                                                    // to this point if we have finished the safe hold timer.
    rwMtxWUnlock(&State::Master::master_state_lock);
    chThdExit((msg_t)0);
}