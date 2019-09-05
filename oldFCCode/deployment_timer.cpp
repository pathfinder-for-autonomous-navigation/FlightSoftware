/** @file deployment_timer.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains the deployment timer code, which is responsible for timing
 * and notifying all threads when deployment timing is complete.
 */

#include "deployment_timer.hpp"
#include <EEPROM.h>
#include "controllers/controllers.hpp"
#include "debug.hpp"
#include "state/EEPROMAddresses.hpp"
#include "state/state_holder.hpp"

//! Working area for deployment timer thread.
THD_WORKING_AREA(deployment_timer_workingArea, 4096);
threads_queue_t deployment_timer_waiting;

void exit_deployment_timer() {
    State::write(State::Master::is_deployed, true, State::Master::master_state_lock);
    chMtxLock(&eeprom_lock);
    EEPROM.put(EEPROM_ADDRESSES::DEPLOYMENT, true);
    chMtxUnlock(&eeprom_lock);

    if (!State::Hardware::check_is_functional(Devices::dcdc)) {
        chMtxLock(&State::Hardware::dcdc_device_lock);
        Devices::dcdc->enable();
        chMtxUnlock(&State::Hardware::dcdc_device_lock);
    }
    if (!State::Hardware::check_is_functional(Devices::spike_and_hold)) {
        chMtxLock(&State::Hardware::spike_and_hold_device_lock);
        Devices::spike_and_hold->enable();
        chMtxUnlock(&State::Hardware::spike_and_hold_device_lock);
    }

    dbg.println(debug_severity::INFO,
                "Notifying all processes waiting on deployment timer that timer "
                "has been completed.");
    chSysLock();
    chThdDequeueAllI(&deployment_timer_waiting, MSG_OK);
    chSysUnlock();
    dbg.println(debug_severity::INFO, "Process terminating.");
    chThdExit((msg_t)0);
}

//! Function that defines the deployment timer thread.
void deployment_timer_function(void *arg) {
    chRegSetThreadName("startup.deployment_timer");

    chMtxLock(&eeprom_lock);
    bool deployed = (EEPROM.read(EEPROM_ADDRESSES::DEPLOYMENT) == 1);
    chMtxUnlock(&eeprom_lock);
    if (deployed) {
        dbg.println(debug_severity::NOTICE,
                    "Deployment hold time constraint has "
                    "been met. Skipping deployment timer.");
        exit_deployment_timer();
    } else
        dbg.println(debug_severity::NOTICE,
                    "Deployment hold time constraint has not been met. "
                    "Initializing deployment timer.");

    dbg.println(debug_severity::NOTICE, "Deployment timer has been started.");

    // Determine time remaining in deployment
    chMtxLock(&eeprom_lock);
    unsigned int time_elapsed;
    EEPROM.get(EEPROM_ADDRESSES::DEPLOYMENT_TIMER, time_elapsed);
    chMtxUnlock(&eeprom_lock);

    // Start deployment timer
    while (time_elapsed < DEPLOYMENT_LENGTH) {
        chMtxLock(&eeprom_lock);
        EEPROM.put(EEPROM_ADDRESSES::DEPLOYMENT_TIMER, time_elapsed);
        chMtxUnlock(&eeprom_lock);

        dbg.printf(debug_severity::NOTICE, "Time remaining until deployment wait completed: %d",
                   DEPLOYMENT_LENGTH - time_elapsed);
        chThdSleepSeconds(1);
        time_elapsed++;
    }

    dbg.println(debug_severity::NOTICE,
                "Deployment timer completed! Writing achievement to permanent memory.");
    exit_deployment_timer();
}