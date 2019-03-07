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
#include <bitset>

using Devices::piksi;
using State::Piksi::piksi_state_lock;

namespace RTOSTasks {
    THD_WORKING_AREA(piksi_controller_workingArea, 2048);
}

static void piksi_read() {
    debug_println("Reading Piksi data");

    // GPS Time
    gps_time_t current_time;
    piksi.get_gps_time(&current_time);
    if (current_time != State::read(State::Piksi::recorded_current_time, piksi_state_lock))
        State::write(State::Piksi::recorded_current_time, current_time, piksi_state_lock);

    // GPS Position
    std::array<double, 3> pos;
    unsigned int pos_tow;
    piksi.get_pos_ecef(&pos, &pos_tow);
    gps_time_t pos_time = current_time;
    pos_time.gpstime.tow = pos_tow;
    unsigned char pos_nsats = piksi.get_pos_ecef_nsats();
    if (pos != State::Piksi::recorded_gps_position) {
        State::write(State::Piksi::recorded_gps_position, pos, piksi_state_lock);
        State::write(State::Piksi::recorded_gps_position_time, pos_time, piksi_state_lock);
        State::write(State::Piksi::recorded_gps_position_nsats, pos_nsats, piksi_state_lock);
    }
    std::bitset<8> pos_ecef_flags(piksi.get_pos_ecef_flags());

    // GPS other position. We only record this if we're actually getting
    // RTK data (which we can know from the above-defined flags); otherwise, we 
    // let data uplinks update this value for us.
    if (pos_ecef_flags[0] == 0) {
        std::array<double, 3> pos_other;
        piksi.get_base_pos_ecef(&pos_other);
        gps_time_t pos_other_time = current_time;
        pos_other_time.gpstime.tow = pos_tow;
        if (pos != State::Piksi::recorded_gps_position_other) {
            State::write(State::Piksi::recorded_gps_position_other, pos_other, piksi_state_lock);
            State::write(State::Piksi::recorded_gps_position_other_time, pos_other_time, piksi_state_lock);
        }
    }

    // GPS Velocity
    std::array<double, 3> vel;
    unsigned int vel_tow;
    piksi.get_vel_ecef(&pos, &vel_tow);
    gps_time_t vel_time = current_time;
    vel_time.gpstime.tow = vel_tow;
    unsigned char vel_nsats = piksi.get_vel_ecef_nsats();
    if (vel != State::Piksi::recorded_gps_velocity) {
        State::write(State::Piksi::recorded_gps_velocity, vel, piksi_state_lock);
        State::write(State::Piksi::recorded_gps_velocity_time, vel_time, piksi_state_lock);
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

        if (State::Hardware::can_get_data(Devices::piksi)) {
            piksi_read();
        }

        chThdSleepUntil(time);
    }
}
