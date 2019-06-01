/** @file quake.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for the Quake state controller.
 */

#include <EEPROM.h>
#include "../controllers.hpp"
#include "../constants.hpp"
#include "../../state/EEPROMAddresses.hpp"
#include "../../state/state_holder.hpp"
#include "../gomspace/power_cyclers.hpp"
#include "../../deployment_timer.hpp"
#include "transceiving_thread.hpp"

namespace RTOSTasks {
    THD_WORKING_AREA(quake_controller_workingArea, 4096);
}
using State::Quake::QuakeState;
using State::Quake::quake_state_lock;
using Devices::quake;
using namespace Comms;

static virtual_timer_t waiting_timer;
static void end_waiting(void* args) {
    chSysLockFromISR();
        State::Quake::quake_state = QuakeState::TRANSCEIVING;
    chSysUnlockFromISR();
}

static CH_IRQ_HANDLER(network_ready_handler) {
    CH_IRQ_PROLOGUE();
    chSysLockFromISR();
        chVTResetI(&waiting_timer);
        State::Quake::quake_state = QuakeState::TRANSCEIVING;
    chSysUnlockFromISR();
    CH_IRQ_EPILOGUE();
};

static void quake_loop() {
    // PCYCLER:QUAKE if failing. Do this for as many times as it takes for the device
    // to start talking again.
    if (!State::Hardware::check_is_functional(quake) && Gomspace::quake_thread == NULL) {
        // Specify arguments for thread
        Gomspace::cycler_arg_t cycler_args = {
            &State::Hardware::quake_device_lock,
            quake,
            Devices::Gomspace::DEVICE_PINS::QUAKE
        };
        // Start cycler thread
        Gomspace::quake_thread = chThdCreateFromMemoryPool(&Gomspace::power_cycler_pool,
            "PCYCLER:QUAKE",
            RTOSTasks::master_thread_priority,
            Gomspace::cycler_fn, (void*) &cycler_args);
        return;
    }

    chMtxLock(&eeprom_lock);
        unsigned int hours_since_sbdix = State::Quake::msec_since_last_sbdix() / 1000 / 60 / 60;
        EEPROM.put(EEPROM_ADDRESSES::HOURS_SINCE_SBDIX, hours_since_sbdix);
    chMtxUnlock(&eeprom_lock);

    QuakeState quake_state = State::read(State::Quake::quake_state, quake_state_lock);
    switch(quake_state) {
        case QuakeState::WAITING: {
            unsigned int quake_wait_period = Constants::read(Constants::Quake::QUAKE_WAIT_PERIOD);
            chVTDoSetI(&waiting_timer, quake_wait_period, end_waiting, NULL);
        }
        break;
        case QuakeState::TRANSCEIVING: {
            if (Quake::transceiving_thread == NULL || chThdTerminatedX(Quake::transceiving_thread)) {
                Quake::transceiving_thread = chThdCreateStatic(Quake::transceiving_thread_workingArea, 
                                                               sizeof(Quake::transceiving_thread_workingArea), 
                                                               Quake::transceiving_thread_priority, 
                                                               Quake::transceiving_fn,
                                                               NULL);
            }
        };
        break;
        default: {
            Quake::go_to_waiting();
        }
    }
}

void RTOSTasks::quake_controller(void *arg) {
    chRegSetThreadName("QUAKE");
    debug_println("Quake radio controller process has started.");
    chVTObjectInit(&waiting_timer);
    attachInterrupt(quake->nr_pin(), network_ready_handler, RISING);
    debug_println("Waiting for deployment timer to finish.");
    
    bool is_deployed = State::read(State::Master::is_deployed, State::Master::master_state_lock);
    if (!is_deployed) chThdEnqueueTimeoutS(&deployment_timer_waiting, S2ST(DEPLOYMENT_LENGTH));
    debug_println("Deployment timer has finished.");
    debug_println("Initializing main operation...");

    systime_t deadline = chVTGetSystemTimeX();
    while(true) {
        deadline += MS2ST(RTOSTasks::LoopTimes::QUAKE);
        quake_loop();
        chThdSleepUntil(deadline);
    }

    chThdExit((msg_t)0);
}