#include "../../state/state_holder.hpp"
#include "../controllers.hpp"
#include <j8_orbit.hpp>

namespace RTOSTasks {
    THD_WORKING_AREA(obp_controller_workingArea, 2048);
}

using State::GNC::gnc_state_lock;
J8Buffer scratchpad;

bool in_nighttime = false;
static void propagate_self_orbit() {
    rwMtxRLock(&gnc_state_lock);
        std::array<double, 3> gps_pos = State::GNC::gps_position;
        std::array<double, 3> gps_vel = State::GNC::gps_velocity;
        j8_propagate(gps_pos, gps_vel, RTOSTasks::LoopTimes::ORBIT_PROPAGATOR, scratchpad);
    rwMtxRUnlock(&State::GNC::gnc_state_lock);
    rwMtxWLock(&gnc_state_lock);
        State::GNC::gps_position = gps_pos;
        State::GNC::gps_velocity = gps_vel;
    rwMtxWUnlock(&State::GNC::gnc_state_lock);

    bool now_in_nighttime = false;
    if (in_nighttime && !now_in_nighttime) {
        in_nighttime = false;
        rwMtxWLock(&State::GNC::gnc_state_lock);
            // Since we're leaving nighttime, we don't have to worry about the fact that
            // we may be manuevering at night
            State::GNC::has_firing_happened_in_nighttime = false;
        rwMtxWUnlock(&State::GNC::gnc_state_lock);
    }
    else if (!in_nighttime && now_in_nighttime) {
        in_nighttime = true;
    }
}

static void update_rotation_quaternion() {
    rwMtxWLock(&gnc_state_lock);
        State::GNC::ecef_to_eci[0] = 0;
    rwMtxWLock(&gnc_state_lock);
}

static void propagate_other_orbit() {
    rwMtxRLock(&gnc_state_lock);
        std::array<double, 3> gps_pos = State::GNC::gps_position_other;
        std::array<double, 3> gps_vel = State::GNC::gps_velocity_other;
        j8_propagate(gps_pos, gps_vel, RTOSTasks::LoopTimes::ORBIT_PROPAGATOR, scratchpad);
    rwMtxRUnlock(&State::GNC::gnc_state_lock);
    rwMtxWLock(&gnc_state_lock);
        State::GNC::gps_position_other = gps_pos;
        State::GNC::gps_velocity_other = gps_vel;
    rwMtxWUnlock(&State::GNC::gnc_state_lock);
}

// TODO matt walsh

void orbit_propagator_controller(void* args) {
    systime_t time = chVTGetSystemTimeX();
    while (true) {
        time += MS2ST(RTOSTasks::LoopTimes::ORBIT_PROPAGATOR);
        propagate_self_orbit();
        update_rotation_quaternion();
        propagate_other_orbit();
        chThdSleepUntil(time);
    }
}