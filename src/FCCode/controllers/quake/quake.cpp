/** @file quake.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for the Quake state controller.
 */

#include "../controllers.hpp"
#include "../../state/state_holder.hpp"
#include "../../deployment_timer.hpp"
#include "../../comms/downlink_serializer.hpp"
#include "../../comms/uplink_deserializer.hpp"
#include "downlink_thread.hpp"
#include <HardwareSerial.h>

namespace RTOSTasks {
    THD_WORKING_AREA(quake_controller_workingArea, 4096);
}
using State::Quake::QuakeState;
using State::Quake::quake_state_lock;
using namespace Comms;

static virtual_timer_t waiting_timer;
static void end_waiting(void* args) {
    chSysLockFromISR();
        // TODO notify the producer threads to stop and dump packet into stack
        State::Quake::quake_state = QuakeState::TRANSCEIVING;
    chSysUnlockFromISR();
}

static CH_IRQ_HANDLER(network_ready_handler) {
    CH_IRQ_PROLOGUE();
    chVTResetI(&waiting_timer);
    chSysLockFromISR();
        State::Quake::quake_state = QuakeState::TRANSCEIVING;
    chSysLockFromISR();
    CH_IRQ_EPILOGUE();
};

static void quake_loop() {
    rwMtxRLock(&quake_state_lock);
        QuakeState quake_state = State::Quake::quake_state;
    rwMtxRUnlock(&quake_state_lock);
    switch(quake_state) {
        case QuakeState::WAITING: {
            chVTDoSetI(&waiting_timer, Constants::Quake::QUAKE_WAIT_PERIOD, end_waiting, NULL);
        }
        break;
        case QuakeState::TRANSCEIVING: {
            if (Quake::downlink_thread == NULL || chThdTerminatedX(Quake::downlink_thread)) {
                Quake::downlink_thread = chThdCreateStatic(Quake::downlink_thread_workingArea, 
                                                           sizeof(Quake::downlink_thread_workingArea), 
                                                           Quake::downlink_thread_priority, 
                                                           Quake::downlink_fn, NULL);
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
    attachInterrupt(Devices::quake.nr_pin(), network_ready_handler, RISING);
    debug_println("Waiting for deployment timer to finish.");
    rwMtxRLock(&State::Master::master_state_lock);
        bool is_deployed = State::Master::is_deployed;
    rwMtxRUnlock(&State::Master::master_state_lock);
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