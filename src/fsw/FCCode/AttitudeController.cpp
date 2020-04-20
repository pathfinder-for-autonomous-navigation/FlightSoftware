#include "AttitudeController.hpp"

#include "adcs_state_t.enum"

#include <gnc/constants.hpp>
#include <gnc/environment.hpp>
#include <gnc/utilities.hpp>

#include <lin/core.hpp>
#include <lin/generators/constants.hpp>
#include <lin/math.hpp>
#include <lin/queries.hpp>
#include <lin/references.hpp>

using namespace gnc;

AttitudeController::AttitudeController(StateFieldRegistry &registry, unsigned int offset) :
        TimedControlTask<void>(registery, offset),
        b_body_rd_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.mag_vec)),
        b_body_est_fp(FIND_READABLE_FIELD(lin::Vector3f, attitude_estimator.b_body)),
        s_body_est_fp(FIND_READABLE_FIELD(lin::Vector3f, attitude_estimator.s_body)),
        q_body_eci_est_fp(FIND_READABLE_FIELD(lin::Vector4f, attitude_estimator.q_body_eci)),
        w_body_est_fp(FIND_READABLE_FIELD(lin::Vector3f, attitude_estimator.w_body)),
        adcs_state_fp(FIND_WRITABLE_FIELD(unsigned char, adcs.state)),
        pos_ecef_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit.pos_ecef)),
        vel_ecef_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit.vel_ecef)),
        pos_baseline_ecef_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit.pos_baseline_ecef)),
        pan_time_fp(FIND_READABLE_FIELD(double, pan.time)),
        pointer_vec1_current_f("attitude.pointer_vec1_current", Serializer<lin::Vector3f>(0, 1, 100)),
        pointer_vec1_desired_f("attitude.pointer_vec1_desired", Serializer<lin::Vector3f>(0, 1, 100)),
        pointer_vec2_current_f("attitude.pointer_vec2_current", Serializer<lin::Vector3f>(0, 1, 100)),
        pointer_vec2_desired_f("attitude.pointer_vec2_desired", Serializer<lin::Vector3f>(0, 1, 100)),
        t_body_cmd_f("attitude.t_body_cmd", Serializer<lin::Vector3f>()),
        m_body_cmd_f("attitude.m_body_cmd", Serializer<lin::Vector3f>()),
        detumbler_state(),
        pointer_state() {
    // Add all new readable state fields
    add_readable_field(pointer_vec1_current_f);
    add_readable_field(pointer_vec1_desired_f);
    add_readable_field(t_body_cmd_f);
    add_readable_field(m_body_cmd_f);

    // Add all new writable state fields
    add_writable_field(pointer_vec1_desired_f);
    add_writable_field(pointer_vec2_desired_f);

    default_all();
}

void AttitudeController::execute() {
    default_actuator_commands();

    switch (static_cast<adcs_state_t>(adcs_state_fp->get())) {
        /* */
        case adcs_state_t::detumble:
        case adcs_state_t::limited:
            calculate_detumble_controller();
            break;
        /* */
        case adcs_state_t::point_standby:
        case adcs_state_t::point_docking:
            calculate_pointing_objectives();
        case adcs_state_t::point_manual:
            calculate_pointing_controller();
            break;
        /* */
        case adcs_state_t::startup:
        case adcs_state_t::zero_torque:
        case adcs_state_t::zero_L:
        default:
            break;
    }
}

void AttitudeController::default_actuator_commands() {
    t_body_cmd_f.set(lin::nans<lin::Vector3f>());
    m_body_cmd_t.set(lin::nans<lin::Vector3f>());
}

void AttitudeController::default_pointing_objectives() {
    pointer_vec1_current_f.set(lin::nans<lin::Vector3f>());
    pointer_vec1_desired_f.set(lin::nans<lin::Vector3f>());
    pointer_vec2_current_f.set(lin::nans<lin::Vector3f>());
    pointer_vec2_desired_f.set(lin::nans<lin::Vector3f>());
}

void AttitudeController::default_all() {
    default_actuator_commands();
    default_pointing_objectives();
}

void AttitudeController::calculate_detumble_controller() {
    m_body_cmd_f.set(lin::zeros<lin::Vector3f>());

    // Default all inputs to NaNs and set appropriate fields
    detumbler_data = DetumbleControllerData();
    detumbler_data.b_body = b_body_rd_fp->get();

    // Call the controller and write results to appropriate state fields
    control_detumble(detumbler_state, detumbler_data, detumbler_actuation);
    if (lin::all(lin::isfinite(detumbler_actuation.mtr_body_cmd)))
        m_body_cmd_f.set(detumbler_actuation.mtr_body_cmd);
}

void AttitudeController::calculate_pointing_objectives() {
    default_pointing_objectives();

    lin::Vector3f dr_body = lin::nans<lin::Vector3f>();
    lin::Matrix3x3f DCM_hill_body = lin::nans<lin::Matrix3x3f>();
    {
        lin::Vector3f r = pos_ecef_fp->get(); // r = r_ecef
        lin::Vector3f v = vel_ecef_fp->get(); // v = v_ecef

        // Position and velocity must be finite
        if (lin::any(!(lin::isfinite(r) && lin::isfinite(v)))) return;

        // Current time since the PAN epoch in seconds
        double time = static_cast<double>(time_ns_fp->get()) * 1.0e-9;

        lin::Vector4f q_body_ecef;
        env::earth_attitude(time, q_body_ecef); // q_body_ecef = q_ecef_eci
        utl::quat_conj(q_body_ecef);            // q_body_ecef = q_eci_ecef
        utl::quat_cross_mult(q_body_eci_est_fp->get(), q_body_ecef);

        lin::Vector3f w_earth_ecef;
        env::earth_angular_rate(t, w_earth_ecef_eci);  // rate of ecef frame in eci
        v = v - lin::cross(w_earth_ecef, r);           // v_ecef but intertial

        utl::rotate_frame(q_body_ecef, r); // Throw the vectors into the body frame
        utl::rotate_frame(q_body_ecef, v);
        utl::dcm(DCM_hill_body, r, v);     // Calculate our dcm

        lin::Vector3f dr = pos_baseline_ecef_fp->get(); // dr = dr_ecef

        // Ensure we have a valid relative position
        if (lin::all(lin::isfinite(dr_body))) {
            dr_body = dr;                            // dr_body = dr_ecef
            utl::rotate_frame(q_body_ecef, dr_body);
        }
    }

    switch (static_cast<adcs_state_t>(adcs_state_fp->get())) {
        /* The general idea for standby pointing is to have the antenna face
         * along the velocity vector (we're assuming a near circular orbt here)
         * and have the docking face pointing normal to our orbit. */
        case adcs_state_t::point_standy:
            // Ensure we have a DCM and time
            if (lin::any(!lin::isfinite(DCM_hill_body)))
                return;
            pointer_vec1_current_f.set({1.0f, 0.0f, 0.0f}); // Antenna face
            pointer_vec2_current_f.set({0.0f, 0.0f, 1.0f}); // Docking face
            pointer_vec1_desired_f.set(lin::ref_col(DCM_hill_body, 1)); // v_hat
            pointer_vec2_desired_f.set(lin::ref_col(DCM_hill_body, 2)); // n_hat
            break;
        /* Here we simply want to point the docking face towards the other
         * satellte and then try to keep GPS for RTK purposes. */
        case adcs_state_t::point_docking: {
            if (lin::any(!lin::isfinite(DCM_hill_body)) || lin::any(!lin::isfinite(dr_body)))
                return;
            pointer_vec1_current_f.set({0.0f, 0.0f, 1.0f}); // Docking face
            pointer_vec2_current_f.set({1.0f, 0.0f, 0.0f}); // Antenna face
            pointer_vec1_desired_f.set(dr_body / lin::norm(dr_body));   // dr_hat
            pointer_vec2_desired_f.set(lin::ref_col(DCM_hill_body, 2)); // n_hat
            break;
        }
        /* In other adcs states, we won't specify a pointing strategy. */
        default:
            break;
    }
}

void AttitudeController::calculate_pointing_controller() {
    // Default all inputs to NaNs and set appropriate fields
    pointer_data = PointingControllerData();
    pointer_data.primary_desired   = pointer_vec1_desired_f.get();
    pointer_data.primary_current   = pointer_vec1_current_f.get();
    pointer_data.secondary_desired = pointer_vec2_desired_f.get();
    pointer_data.secondary_current = pointer_vec2_current_f.get();
    pointer_data.w_wheels          = w_wheels_rd_fp->get();
    pointer_data.w_sat             = w_body_est_fp->get();
    pointer_data.b                 = b_body_est_fp->get();

    // Call the controller and write results to appropriate state fields
    control_pointing(pointer_state, pointer_data, pointer_actuation);
}
