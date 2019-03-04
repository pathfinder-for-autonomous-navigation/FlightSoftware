/** @file controllers/piksi.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for the Piksi GPS state controller.
 */

#include "controllers.hpp"
#include "../state/state_holder.hpp"
#include "../deployment_timer.hpp"
#include "../data_collection/data_collection.hpp"
#include <Piksi/Piksi.hpp>

namespace RTOSTasks {
    THD_WORKING_AREA(piksi_controller_workingArea, 2048);
}

static void piksi_read() {
    debug_println("Reading Piksi data");
}

static void piksi_check() {
    debug_println("Checking Piksi data");
}

void piksi_write_defaults() {
    debug_println("Writing default values to Piksi.");
}

void RTOSTasks::piksi_controller(void *arg) {
    chRegSetThreadName("PIKSI");
    debug_println("Piksi controller process has started.");

    DataCollection::initialize_piksi_history_timers();
    
    debug_println("Waiting for deployment timer to finish.");
    rwMtxRLock(&State::Master::master_state_lock);
        bool is_deployed = State::Master::is_deployed;
    rwMtxRUnlock(&State::Master::master_state_lock);
    if (!is_deployed) chThdEnqueueTimeoutS(&deployment_timer_waiting, S2ST(DEPLOYMENT_LENGTH));
    debug_println("Deployment timer has finished.");
    debug_println("Initializing main operation...");

    systime_t time = chVTGetSystemTimeX();
    while(true) {
        time += MS2ST(RTOSTasks::LoopTimes::PIKSI);
        piksi_read();
        piksi_check();
        chThdSleepUntil(time);
    }
}
