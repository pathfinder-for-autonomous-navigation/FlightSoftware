#include "../../state/state_holder.hpp"
#include "../controllers.hpp"
#include <j8_orbit.hpp>
#include "orbit_propagators.hpp"
#include <tensor.hpp>

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

static void propagate_other_orbit() {
    std::array<double, 3> gps_pos = State::read(State::GNC::gps_position_other, gnc_state_lock);
    std::array<double, 3> gps_vel = State::read(State::GNC::gps_velocity_other, gnc_state_lock);
    j8_propagate(gps_pos, gps_vel, 1.0 / GNC::ORBIT_PROPAGATOR_DELTA_T, scratchpad);
    State::write(State::GNC::gps_position_other, gps_pos, gnc_state_lock);
    State::write(State::GNC::gps_velocity_other, gps_vel, gnc_state_lock);
}

static void update_eci_to_ecef() {
    // TODO
}

static void update_eci_to_lvlh() {
    // Compute ECI to LVLH
    std::array<double, 3> v_raw = State::read(State::GNC::gps_velocity, State::GNC::gnc_state_lock);
    std::array<double, 3> r_raw = State::read(State::GNC::gps_position, State::GNC::gnc_state_lock);
    pla::Vec3f v, r;
    for(int i = 0; i < 3; i++) v[i] = v_raw[i];
    for(int i = 0; i < 3; i++) r[i] = r_raw[i];
    
    pla::Vec3f e1 = v.normalize_copy(); // v / |v|
    pla::Vec3f e2; // (r x v)/|r x v|
    vect_cross_mult(r.get_data(), v.get_data(), e2.get_data());
    e2.normalize();
    pla::Vec3f e3; // e1 x e2
    vect_cross_mult(e1.get_data(), e2.get_data(), e3.get_data());
    e3.normalize();
    
    pla::Mat3x3f eci_to_lvlh_dcm;
    for(int i = 0; i < 3; i++) {
        eci_to_lvlh_dcm[0][i] = e1[i];
        eci_to_lvlh_dcm[2][i] = e2[i];
        eci_to_lvlh_dcm[3][i] = e3[i];
    }
    pla::Vec4f eci_to_lvlh_quat;
    quat_from_dcm(eci_to_lvlh_dcm.get_data(), eci_to_lvlh_quat.get_data());
    std::array<float, 4> eci_to_lvlh;
    for(int i = 0; i < 4; i++) eci_to_lvlh[i] = eci_to_lvlh_quat[i];
    State::write(State::GNC::eci_to_lvlh, eci_to_lvlh, State::GNC::gnc_state_lock);
}

// TODO matt walsh big buffer

thread_t* GNC::orbit_propagator_thread;
THD_WORKING_AREA(GNC::orbit_propagator_workingArea, 2048);

THD_FUNCTION(GNC::orbit_propagator_controller, args) {
    systime_t time = chVTGetSystemTimeX();
    while (true) {
        time += MS2ST(GNC::ORBIT_PROPAGATOR_DELTA_T);
        propagate_self_orbit();
        rwMtxWLock(&gnc_state_lock);
            // We're locking these up so that other processes don't update themselves on
            // mismatched ECI/ECEF and ECI/LVLH quaternions!
            update_eci_to_ecef();
            update_eci_to_lvlh();
        rwMtxWLock(&gnc_state_lock);
        propagate_other_orbit();
        chThdSleepUntil(time);
    }
}