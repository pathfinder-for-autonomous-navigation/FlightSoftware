/** @file controllers/piksi.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for the Piksi GPS state controller.
 */

#include "controllers.hpp"
#include "../state/state_holder.hpp"
#include "gomspace/power_cyclers.hpp"
#include "../deployment_timer.hpp"
#include "../data_collection/data_collection.hpp"
#include <Piksi.hpp>
#include <bitset>

using Devices::piksi;
using State::Piksi::piksi_state_lock;

namespace RTOSTasks {
    THD_WORKING_AREA(piksi_controller_workingArea, 2048);
}

static void piksi_read() {
    debug_println("Reading Piksi data");

    // Try to parse Piksi buffer three times
    bool successful = false;
    for(int i = 0; i < 3; i++) {
        successful = Devices::piksi().process_buffer();
        if (successful) break;
        chThdSleepMilliseconds(RTOSTasks::LoopTimes::PIKSI / 5); // Wait ~20 ms
    }
    if (!successful) return; // TODO add error recording

    // GPS Time
    gps_time_t current_time;
    piksi().get_gps_time(&current_time);
    if (current_time != State::read(State::Piksi::recorded_current_time, piksi_state_lock))
        State::write(State::Piksi::recorded_current_time, current_time, piksi_state_lock);

    // GPS Position
    std::array<double, 3> pos;
    unsigned int pos_ns; // TODO add function to Piksi to get nanoseconds as well
    piksi().get_pos_ecef(&pos);
    
    chMtxLock(&State::Hardware::piksi_device_lock);
        unsigned char pos_nsats = piksi().get_pos_ecef_nsats();
        unsigned char pos_flags = piksi().get_pos_ecef_flags();
    chMtxUnlock(&State::Hardware::piksi_device_lock);
    if (pos != State::Piksi::recorded_gps_position) {
        State::write(State::Piksi::recorded_gps_position, pos, piksi_state_lock);
        State::write(State::Piksi::recorded_gps_position_time, current_time, piksi_state_lock);
        State::write(State::Piksi::recorded_gps_position_nsats, pos_nsats, piksi_state_lock);
    }

    // GPS other position. We only record this if we're actually getting
    // RTK data (which we can know from the above-defined flags); otherwise, we 
    // let data uplinks update this value for us.
    // Write flags to state
    if (pos_flags != 0) {
        std::array<double, 3> pos_other;
        piksi().get_base_pos_ecef(&pos_other);
        if (pos_other != State::Piksi::recorded_gps_position_other) {
            State::write(State::Piksi::recorded_gps_position_other, pos_other, piksi_state_lock);
            State::write(State::Piksi::recorded_gps_position_other_time, current_time, piksi_state_lock);
        }
        if (pos_flags == 1) {
            State::write(State::Piksi::is_float_rtk, true, State::Piksi::piksi_state_lock);
            State::write(State::Piksi::is_fixed_rtk, false, State::Piksi::piksi_state_lock);
        }
        else if (pos_flags == 2) {
            State::write(State::Piksi::is_float_rtk, false, State::Piksi::piksi_state_lock);
            State::write(State::Piksi::is_fixed_rtk, true, State::Piksi::piksi_state_lock);
        }
    }
    else {
        State::write(State::Piksi::is_float_rtk, false, State::Piksi::piksi_state_lock);
        State::write(State::Piksi::is_fixed_rtk, false, State::Piksi::piksi_state_lock);
    }

    // GPS Velocity
    std::array<double, 3> vel;
    piksi().get_vel_ecef(&pos);

    unsigned char vel_nsats = piksi().get_vel_ecef_nsats();
    if (vel != State::Piksi::recorded_gps_velocity) {
        State::write(State::Piksi::recorded_gps_velocity, vel, piksi_state_lock);
        State::write(State::Piksi::recorded_gps_velocity_time, current_time, piksi_state_lock);
        State::write(State::Piksi::recorded_gps_velocity_nsats, vel_nsats, piksi_state_lock);
    }
}

void RTOSTasks::piksi_controller(void *arg) {
    chRegSetThreadName("PIKSI");
    debug_println("Piksi controller process has started.");

    DataCollection::initialize_piksi_history_timers();
    
    debug_println("Waiting for deployment timer to finish.");
    bool is_deployed = State::read(State::Master::is_deployed, State::Master::master_state_lock);
    if (!is_deployed) chThdEnqueueTimeoutS(&deployment_timer_waiting, S2ST(DEPLOYMENT_LENGTH));
    debug_println("Deployment timer has finished.");
    debug_println("Initializing main operation...");

    systime_t time = chVTGetSystemTimeX();
    while(true) {
        time += MS2ST(RTOSTasks::LoopTimes::PIKSI);

        // Power cycle Piksi if failing. Do this for as many times as it takes for the device
        // to start talking again.
        // TODO add option from ground to disable power cycling
        if (!State::Hardware::check_is_functional(&piksi()) && Gomspace::piksi_thread == NULL) {
            // Specify arguments for thread
            Gomspace::cycler_arg_t cycler_args = {
                &State::Hardware::piksi_device_lock,
                &piksi(),
                Devices::Gomspace::DEVICE_PINS::PIKSI
            };
            // Start cycler thread
            Gomspace::piksi_thread = chThdCreateFromMemoryPool(&Gomspace::power_cycler_pool,
                "POWER CYCLE PIKSI",
                RTOSTasks::master_thread_priority,
                Gomspace::cycler_fn, (void*) &cycler_args);
        }
        else if (State::Hardware::check_is_functional(&piksi())) {
            piksi_read();
        }

        chThdSleepUntil(time);
    }
}
