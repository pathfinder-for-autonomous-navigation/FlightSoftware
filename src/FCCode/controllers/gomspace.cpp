/** @file controllers/gomspace.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for the gomspace state controller.
 */

#include "controllers.hpp"
#include "../state/state_holder.hpp"
#include "../deployment_timer.hpp"

using Devices::Gomspace;
using Devices::gomspace;
using State::Gomspace::gomspace_data;

namespace RTOSTasks {
    THD_WORKING_AREA(gomspace_controller_workingArea, 4096);
}

static void gomspace_read() {
    debug_printf("Reading Gomspace data...");
    unsigned char t = 0; // # of tries at reading housekeeping data
    while (t < 5) {
        if (gomspace.get_hk()) break;
        t++;
    }
    if (t == 5) {
        debug_println("unable to read Gomspace data.");
        rwMtxWLock(&State::Hardware::hat_lock);
            (State::Hardware::hat).at("Gomspace").is_functional = false;
        rwMtxWUnlock(&State::Hardware::hat_lock);
    }
    else debug_printf("battery voltage (mV): %d\n", gomspace_data.vbatt);
}

static void gomspace_check() {
    debug_println("Checking Gomspace data...");
    
    debug_printf("Checking if Gomspace is functional...");
    rwMtxRLock(&State::Hardware::hat_lock);
    bool is_gomspace_functional = (State::Hardware::hat).at("Gomspace").is_functional;
    rwMtxRUnlock(&State::Hardware::hat_lock);
    if (!is_gomspace_functional) {
        debug_println("Gomspace is not functional!");
        return;
    }
    else debug_println("Device is functional.");

    debug_printf("Checking Gomspace battery voltage...");
    unsigned short int vbatt = gomspace_data.vbatt;
    if (vbatt < Gomspace::VOLTAGES::FC_SAFE) {
        debug_println("Battery voltage is enough for safe hold!");
        // TODO Set some flag for master controller to pick up on
    }

    // TODO
    debug_println("Checking Gomspace inputs (currents and voltages).");
    debug_println("Checking Gomspace outputs (currents and voltages).");
    debug_println("Checking Gomspace temperature.");
}

void set_power_outputs() {
    rwMtxRLock(&State::Master::master_state_lock);
        unsigned int boot_number = State::Master::boot_number;
    rwMtxRUnlock(&State::Master::master_state_lock);

    bool low_power_state = false; // TODO get from Gomspace. If unavailable, _assume_ a low-power state
                                  // since it may be the case that Gomspace is unable to initialize I2C properly
    if (boot_number == 1 || low_power_state) {
        rwMtxWLock(&State::Hardware::hat_lock);
        // TODO turn on Quake
        rwMtxWUnlock(&State::Hardware::hat_lock);
    }
    else {
        // Turn everything on
    }
}

static THD_WORKING_AREA(gomspace_read_controller_workingArea, 4096);
static void gomspace_read_controller(void *arg) {
    chRegSetThreadName("GS READ");
    debug_println("Starting Gomspace reading and checking process.");

    systime_t time = chVTGetSystemTimeX(); // T0
    while (true) {
        time += MS2ST(RTOSTasks::LoopTimes::GOMSPACE);
        gomspace_read();
        gomspace_check();
        chThdSleepUntil(time);
    }
}

void RTOSTasks::gomspace_controller(void *arg) {
    chRegSetThreadName("GOMSPACE");
    debug_println("Gomspace controller process has started.");
    (void)chThdCreateStatic(gomspace_read_controller_workingArea, 
        sizeof(gomspace_read_controller_workingArea),
        RTOSTasks::gomspace_thread_priority, gomspace_read_controller, NULL);
    
    debug_println("Waiting for deployment timer to finish.");
    rwMtxRLock(&State::Master::master_state_lock);
        bool is_deployed = State::Master::is_deployed;
    rwMtxRUnlock(&State::Master::master_state_lock);
    if (!is_deployed) chThdEnqueueTimeoutS(&deployment_timer_waiting, S2ST(DEPLOYMENT_LENGTH));
    debug_println("Deployment timer has finished.");
    debug_println("Initializing main operation...");
    // TODO start actuation control operations? e.g. power on Quake maybe
    chThdExit((msg_t)0);
}