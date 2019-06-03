/** @file controllers/piksi.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for the Piksi GPS state controller.
 */

#include "../controllers.hpp"
#include "../../state/state_holder.hpp"
#include "../../deployment_timer.hpp"
#include <bitset>

using Devices::system_output;
using State::Hardware::system_output_device_lock;
using State::GNC::gnc_state_lock;
using State::Propulsion::firing_data;
using State::Propulsion::propulsion_state_lock;

namespace RTOSTasks {
    THD_WORKING_AREA(system_output_controller_workingArea, 2048);
}

static void send_data() {
    double impulse[3], gps_position[3], gps_velocity[3], gps_position_other[3], gps_velocity_other[3];

    float *impulse_f = State::read(firing_data.impulse_vector.data(), propulsion_state_lock);
    double *gps_pos = State::read(State::GNC::gps_position.data(), gnc_state_lock);
    double *gps_vel = State::read(State::GNC::gps_velocity.data(), gnc_state_lock);
    double *gps_pos_other = State::read(State::GNC::gps_position_other.data(), gnc_state_lock);
    double *gps_vel_other = State::read(State::GNC::gps_velocity_other.data(), gnc_state_lock);
    for(int i = 0; i < 3; i++) {
        impulse[i] = impulse_f[i];
        gps_position[i] = gps_pos[i];
        gps_velocity[i] = gps_vel[i];
        gps_position_other[i] = gps_pos_other[i];
        gps_velocity_other[i] = gps_vel_other[i];
    }

    chMtxLock(&system_output_device_lock);
        system_output->send_impulse(impulse);
        system_output->send_propagated_position(gps_position);
        system_output->send_propagated_velocity(gps_velocity);
        system_output->send_propagated_other_position(gps_position_other);
        system_output->send_propagated_other_velocity(gps_velocity_other);
    chMtxUnlock(&system_output_device_lock);
}

void RTOSTasks::system_output_controller(void *arg) {
    chRegSetThreadName("system_output");
    dbg.println(debug_console::severity::INFO, "System output controller process has started.");
    
    dbg.println(debug_console::severity::INFO, "Waiting for deployment timer to finish.");
    bool is_deployed = State::read(State::Master::is_deployed, State::Master::master_state_lock);
    if (!is_deployed) chThdEnqueueTimeoutS(&deployment_timer_waiting, S2ST(DEPLOYMENT_LENGTH));
    dbg.println(debug_console::severity::INFO, "Deployment timer has finished.");
    dbg.println(debug_console::severity::INFO, "Initializing main operation...");

    systime_t time = chVTGetSystemTimeX();
    while(true) {
        time += MS2ST(RTOSTasks::LoopTimes::SYSTEM_OUTPUT);
        send_data();
        chThdSleepUntil(time);
    }
}
