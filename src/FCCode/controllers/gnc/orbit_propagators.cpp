#include "../../state/state_holder.hpp"
#include "../controllers.hpp"
#include <j8_orbit.hpp>
#include "orbit_propagators.hpp"

using State::GNC::gnc_state_lock;
J8Buffer scratchpad;

bool in_nighttime = false;
static void propagate_self_orbit() {
    std::array<double, 3> gps_pos = State::read(State::GNC::gps_position, gnc_state_lock);
    std::array<double, 3> gps_vel = State::read(State::GNC::gps_velocity, gnc_state_lock);
    j8_propagate(gps_pos, gps_vel, 1.0 / GNC::ORBIT_PROPAGATOR_DELTA_T, scratchpad);
    State::write(State::GNC::gps_position, gps_pos, gnc_state_lock);
    State::write(State::GNC::gps_velocity, gps_vel, gnc_state_lock);

    bool now_in_nighttime = false;
    if (in_nighttime && !now_in_nighttime) {
        in_nighttime = false;
            // Since we're leaving nighttime, we don't have to worry about the fact that
            // we may be manuevering at night
            State::write(State::GNC::has_firing_happened_in_nighttime, false, gnc_state_lock);
    }
    else if (!in_nighttime && now_in_nighttime) {
        in_nighttime = true;
    }
}

static void update_rotation_quaternions() {
    // TODO update ECI to ECEF
    // TODO compute ECI to LVLH
}

static void propagate_other_orbit() {
    std::array<double, 3> gps_pos = State::read(State::GNC::gps_position_other, gnc_state_lock);
    std::array<double, 3> gps_vel = State::read(State::GNC::gps_velocity_other, gnc_state_lock);
    j8_propagate(gps_pos, gps_vel, 1.0 / GNC::ORBIT_PROPAGATOR_DELTA_T, scratchpad);
    State::write(State::GNC::gps_position_other, gps_pos, gnc_state_lock);
    State::write(State::GNC::gps_velocity_other, gps_vel, gnc_state_lock);
}

// TODO matt walsh big buffer

thread_t* GNC::orbit_propagator_thread;
THD_WORKING_AREA(GNC::orbit_propagator_workingArea, 2048);

THD_FUNCTION(GNC::orbit_propagator_controller, args) {
    systime_t time = chVTGetSystemTimeX();
    while (true) {
        time += MS2ST(GNC::ORBIT_PROPAGATOR_DELTA_T);
        propagate_self_orbit();
        update_rotation_quaternions();
        propagate_other_orbit();
        chThdSleepUntil(time);
    }
}