/** @file deployment_timer.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains the deployment timer code, which is responsible for timing and notifying
 * all threads when deployment timing is complete.
 */

#include <EEPROM.h>
#include "../../state/EEPROMAddresses.hpp"
#include "../../state/state_holder.hpp"
#include "master_helpers.hpp"
#include "../constants.hpp"
#include "../../debug.hpp"

namespace Master {
    thread_t* safe_hold_timer_thread;
    THD_WORKING_AREA(safe_hold_timer_workingArea, 2048);
}

//! Function that defines the safe hold timer thread.
void Master::safe_hold_timer(void *arg) {
    chRegSetThreadName("master.safehold_timer");
    // Determine time remaining in safe hold
    chMtxLock(&eeprom_lock);
        unsigned char time_elapsed;
        EEPROM.get(EEPROM_ADDRESSES::SAFE_HOLD_TIMER, time_elapsed);
    chMtxUnlock(&eeprom_lock);

    // Start safe hold timer
    unsigned int safe_hold_timeout = State::read(Constants::Master::SAFE_HOLD_TIMEOUT, Constants::changeable_constants_lock);
    while(time_elapsed < safe_hold_timeout) {
        // If no longer in safe hold, don't keep running down the timer!
        if (State::read(State::Master::master_state, State::Master::master_state_lock) 
                != State::Master::MasterState::SAFE_HOLD) {
            break;
        }

        chMtxLock(&eeprom_lock);
            EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_TIMER, time_elapsed);
        chMtxUnlock(&eeprom_lock);

        dbg.printf(debug_severity::INFO, "Time remaining until safe hold wait completed: %d", safe_hold_timeout - time_elapsed);
        chThdSleepSeconds(1);
        time_elapsed++;
    }

    // Here we set autoexit to true, since we can only get to this point if we have finished the safe hold timer.
    State::write(State::Master::autoexited_safe_hold, true, State::Master::master_state_lock);

    stop_safe_hold(); 
    chThdExit((msg_t)0);
}