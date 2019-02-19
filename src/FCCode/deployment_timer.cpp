/** @file deployment_timer.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains the deployment timer code, which is responsible for timing and notifying
 * all threads when deployment timing is complete.
 */

#include <EEPROM.h>
#include "state/EEPROMAddresses.hpp"
#include "state/state_holder.hpp"
#include "controllers/controllers.hpp"
#include "debug.hpp"
#include "deployment_timer.hpp"

//! Working area for deployment timer thread.
THD_WORKING_AREA(deployment_timer_workingArea, 4096);
threads_queue_t deployment_timer_waiting;

void exit_deployment_timer() {
    rwMtxWLock(&State::Master::master_state_lock);
        State::Master::is_deployed = true;
    rwMtxWUnlock(&State::Master::master_state_lock);
    chMtxLock(&eeprom_lock);
        EEPROM.put(EEPROM_ADDRESSES::DEPLOYMENT, true);
    chMtxUnlock(&eeprom_lock);

    chMtxLock(&State::Hardware::dcdc_lock);
        Devices::dcdc.enable();
    chMtxUnlock(&State::Hardware::dcdc_lock);
    chMtxLock(&State::Hardware::spike_and_hold_lock);
        Devices::spike_and_hold.enable();
    chMtxUnlock(&State::Hardware::spike_and_hold_lock);

    debug_println("Notifying all processes waiting on deployment timer that timer has been completed.");
    chSysLock();
        chThdDequeueAllI(&deployment_timer_waiting, MSG_OK);
    chSysUnlock();
    debug_println("Process terminating.");
    chThdExit((msg_t)0);
}

//! Function that defines the deployment timer thread.
void deployment_timer_function(void *arg) {
    chRegSetThreadName("DEPLOYMENT");
    chThdQueueObjectInit(&deployment_timer_waiting);

    chMtxLock(&eeprom_lock);
        bool deployed = (EEPROM.read(EEPROM_ADDRESSES::DEPLOYMENT) == 1);
    chMtxUnlock(&eeprom_lock);
    if (deployed) {
        debug_println("Deployment hold time constraint has been met. Skipping deployment timer.");
        exit_deployment_timer();
    }
    else debug_println("Deployment hold time constraint has not been met. Initializing deployment timer.");

    debug_println("Deployment timer has been started.");

    // Determine time remaining in deployment
    chMtxLock(&eeprom_lock);
        unsigned int time_elapsed;
        EEPROM.get(EEPROM_ADDRESSES::DEPLOYMENT_TIMER_1, time_elapsed);
    chMtxUnlock(&eeprom_lock);

    // Start deployment timer
    while(time_elapsed < DEPLOYMENT_LENGTH) {
        chMtxLock(&eeprom_lock);
            EEPROM.put(EEPROM_ADDRESSES::DEPLOYMENT_TIMER_1, time_elapsed);
        chMtxUnlock(&eeprom_lock);

        debug_printf("Time remaining until deployment wait completed: %d\n", DEPLOYMENT_LENGTH - time_elapsed);
        chThdSleepSeconds(1);
        time_elapsed++;
    }

    debug_println("Deployment timer completed! Writing achievement to permanent memory.");
    exit_deployment_timer();
}