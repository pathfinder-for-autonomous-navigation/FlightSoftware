#include "AttitudeController.hpp"

#include "adcs_state_t.enum"

#include <adcs/constants.hpp>

#include <gnc/constants.hpp>
#include <gnc/environment.hpp>
#include <gnc/utilities.hpp>

#include <lin/core.hpp>
#include <lin/generators.hpp>
#include <lin/math.hpp>
#include <lin/queries.hpp>
#include <lin/references.hpp>

#include <iostream>

AttitudeController::AttitudeController(StateFieldRegistry &registry) :
    TimedControlTask<void>(registry, "attitude_controller"),
    w_wheels_rd_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.rwa_speed_rd)),
    attitude_estimator_b_valid_fp(FIND_INTERNAL_FIELD(bool, attitude_estimator.b_valid)),
    b_body_rd_fp(FIND_INTERNAL_FIELD(lin::Vector3f, attitude_estimator.b_body)),
    attitude_estimator_valid_fp(FIND_READABLE_FIELD(bool, attitude_estimator.valid)),
    q_body_eci_est_fp(FIND_READABLE_FIELD(lin::Vector4f, attitude_estimator.q_body_eci)),
    w_body_est_fp(FIND_INTERNAL_FIELD(lin::Vector3f, attitude_estimator.w_body)),
    adcs_state_fp(FIND_WRITABLE_FIELD(unsigned char, adcs.state)),
    time_valid_fp(FIND_READABLE_FIELD(bool, time.valid)),
    time_fp(FIND_INTERNAL_FIELD(double, time.s)),
    orbit_valid_fp(FIND_READABLE_FIELD(bool, orbit.valid)),
    pos_ecef_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit.pos)),
    vel_ecef_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit.vel)),
    rel_orbit_state_fp(FIND_READABLE_FIELD(unsigned char, rel_orbit.state)),
    pos_baseline_ecef_fp(FIND_READABLE_FIELD(lin::Vector3d, rel_orbit.rel_pos)),
    unit_vector_sr(1-1e-4,1+1e-4,0),
    pointer_vec1_current_f("attitude.pointer_vec1_current", unit_vector_sr),
    pointer_vec2_current_f("attitude.pointer_vec2_current", unit_vector_sr),
    pointer_vec1_desired_f("attitude.pointer_vec1_desired", unit_vector_sr),
    pointer_vec2_desired_f("attitude.pointer_vec2_desired", unit_vector_sr),
    t_body_cmd_f("pointer.rwa_torque_cmd",
        Serializer<lin::Vector3f>(0.0, 1.73205080757*adcs::rwa::max_torque, 8)),
    m_body_cmd_f("pointer.mtr_cmd",
        Serializer<lin::Vector3f>(0.0, 1.73205080757*adcs::mtr::max_moment, 6)),
    detumbler_state(),
    pointer_state()
    {

    // Add readable current pointing vectors
    add_readable_field(pointer_vec1_current_f);
    add_readable_field(pointer_vec2_current_f);

    // Add writable desired pointing vectors
    add_writable_field(pointer_vec1_desired_f);
    add_writable_field(pointer_vec2_desired_f);

    add_writable_field(t_body_cmd_f);
    add_writable_field(m_body_cmd_f);

    default_actuator_commands();
    default_pointing_objectives();
}

void AttitudeController::execute() {
    auto const state = static_cast<adcs_state_t>(adcs_state_fp->get());
    switch (state) {
        /*
         * While detumbling and in limited attitude control we want to drive our
         * angular rate to zero.
         */
        case adcs_state_t::detumble:
        case adcs_state_t::limited:
            default_actuator_commands();
            calculate_detumble_controller();
            break;
        /*
         * We'll autonomously calculate a pointing objective and then control to
         * it when in standby or docking.
         */
        case adcs_state_t::point_standby:
        case adcs_state_t::point_docking:
            default_actuator_commands();
            default_pointing_objectives();
            calculate_pointing_objectives();
        /*
         * When in point_manual, the pointing objectives are set from the ground.
         * When in manual, we recalculating the pointing_controller is fine too
         */
        case adcs_state_t::point_manual:
        case adcs_state_t::manual:
            calculate_pointing_controller();
            break;
        /*
         * Currently, there is no actuation for the startup, zero torque, or zero 
         * angular momentum states.
         *
         * TODO : Implement control for zero torque and zero L.
         */
        case adcs_state_t::startup:
        case adcs_state_t::zero_torque:
        case adcs_state_t::zero_L:
            default_actuator_commands();
        default:
            break;
    }
}

void AttitudeController::default_actuator_commands() {
    t_body_cmd_f.set(lin::zeros<lin::Vector3f>());
    m_body_cmd_f.set(lin::zeros<lin::Vector3f>());
}

void AttitudeController::default_pointing_objectives() {
    pointer_vec1_current_f.set(lin::nans<lin::Vector3f>());
    pointer_vec1_desired_f.set(lin::nans<lin::Vector3f>());
    pointer_vec2_current_f.set(lin::nans<lin::Vector3f>());
    pointer_vec2_desired_f.set(lin::nans<lin::Vector3f>());
}

void AttitudeController::calculate_detumble_controller() {
    // If the magnetic field from the attitude estimator isn't valid we can't do
    // anything
    if (!attitude_estimator_b_valid_fp->get()) return;

    // Default all inputs to NaNs and set appropriate fields
    detumbler_data = gnc::DetumbleControllerData();
    detumbler_data.b_body = b_body_rd_fp->get();

    // Call the controller and write results to appropriate state fields
    control_detumble(detumbler_state, detumbler_data, detumbler_actuation);
    if (lin::all(lin::isfinite(detumbler_actuation.mtr_body_cmd)))
        m_body_cmd_f.set(detumbler_actuation.mtr_body_cmd);
}

void AttitudeController::calculate_pointing_objectives() {
    // If we don't have a valid time estimate, orbit estimate, or attitude
    // estimator we can't do anything
    if (!time_valid_fp->get() || !attitude_estimator_valid_fp->get() ||
            !orbit_valid_fp->get()) return;

    lin::Vector3f dr_body = lin::nans<lin::Vector3f>();
    lin::Matrix3x3f DCM_hill_body = lin::nans<lin::Matrix3x3f>();
    {
        lin::Vector3f r = pos_ecef_fp->get(); // r = r_ecef
        lin::Vector3f v = vel_ecef_fp->get(); // v = v_ecef
        lin::Vector4f q_body_eci_est = q_body_eci_est_fp->get();

        // Current time since the PAN epoch in seconds
        auto const time_s = static_cast<double>(time_fp->get());

        lin::Vector4f q_body_ecef;
        gnc::env::earth_attitude(time_s, q_body_ecef);          // q_body_ecef = q_ecef_eci
        gnc::utl::quat_conj(q_body_ecef);                       // q_body_ecef = q_eci_ecef
        gnc::utl::quat_cross_mult(q_body_eci_est, q_body_ecef); // q_body_ecef = q_body_ecef

        lin::Vector3f w_earth; // w_earth
        gnc::env::earth_angular_rate(time_s, w_earth); // rate of ecef frame in eci

        v = v + lin::cross(w_earth, r); // v = v_ecef_0, instantaneous inertial

        gnc::utl::rotate_frame(q_body_ecef, r); // r = r_body_0
        gnc::utl::rotate_frame(q_body_ecef, v); // v = v_body_0
        gnc::utl::dcm(DCM_hill_body, r, v);     // Calculate our DCM

        // Ensure we have a valid relative position
        if (rel_orbit_state_fp->get()) {
            dr_body = pos_baseline_ecef_fp->get();        // dr_body = dr_ecef
            gnc::utl::rotate_frame(q_body_ecef, dr_body); // dr_body = dr_body
        }
    }

    adcs_state_t state = static_cast<adcs_state_t>(adcs_state_fp->get());
    switch (state) {
        /*
         * The general idea for standby pointing is to have the antenna face
         * along the velocity vector (we're assuming a near circular orbt here)
         * and have the docking face pointing normal to our orbit.
         */
        case adcs_state_t::point_standby:
            // Ensure we have a DCM and time
            if (lin::any(!lin::isfinite(DCM_hill_body)))
                return;
            pointer_vec1_current_f.set({1.0f, 0.0f,  0.0f});                        // Antenna face
            pointer_vec2_current_f.set({0.0f, 0.0f, -1.0f});                        // Docking face
            pointer_vec1_desired_f.set(lin::transpose(lin::row(DCM_hill_body, 1))); // v_hat_body
            pointer_vec2_desired_f.set(lin::transpose(lin::row(DCM_hill_body, 2))); // n_hat_body
            break;
        /*
         * Here we simply want to point the docking face towards the other
         * satellte and then try to keep GPS for RTK purposes.
         */
        case adcs_state_t::point_docking: {
            if (lin::any(!lin::isfinite(DCM_hill_body)) || lin::any(!lin::isfinite(dr_body)))
                return;
            pointer_vec1_current_f.set({0.0f, 0.0f, -1.0f});                        // Docking face
            pointer_vec2_current_f.set({1.0f, 0.0f,  0.0f});                        // Antenna face
            pointer_vec1_desired_f.set(dr_body / lin::norm(dr_body));               // dr_hat
            pointer_vec2_desired_f.set(lin::transpose(lin::row(DCM_hill_body, 2))); // n_hat_body
            break;
        }
        /* In other adcs states, we won't specify a pointing strategy.
         */
        default:
            break;
    }
}

void AttitudeController::calculate_pointing_controller() {
    // If we don't have a valid orbit estimate or magnetic field reading we
    // can't do anything
    if (!attitude_estimator_valid_fp->get() ||
            !attitude_estimator_b_valid_fp->get()) return;

    // Default all inputs to NaNs and set appropriate fields
    pointer_data = gnc::PointingControllerData();
    pointer_data.primary_desired   = pointer_vec1_desired_f.get();
    pointer_data.primary_current   = pointer_vec1_current_f.get();
    pointer_data.secondary_desired = pointer_vec2_desired_f.get();
    pointer_data.secondary_current = pointer_vec2_current_f.get();
    pointer_data.w_wheels          = w_wheels_rd_fp->get();
    pointer_data.w_sat             = w_body_est_fp->get();
    pointer_data.b                 = b_body_rd_fp->get();

    if ( !lin::all(lin::isfinite(pointer_data.primary_desired)) 
    || !lin::all(lin::isfinite(pointer_data.primary_current)))
        return;

    if ( !lin::all(lin::isfinite(pointer_data.secondary_desired)) 
    || !lin::all(lin::isfinite(pointer_data.secondary_current)))
        return;

    // Call the controller and write results to appropriate state fields
    control_pointing(pointer_state, pointer_data, pointer_actuation);

    // the output of GNC is a desired torque to the spacecraft, so we invert the command
    // to obtain a torque to the wheels to effect the original command
    pointer_actuation.rwa_body_cmd = -1*pointer_actuation.rwa_body_cmd;

    if (lin::all(lin::isfinite(pointer_actuation.mtr_body_cmd) && lin::isfinite(pointer_actuation.rwa_body_cmd))) {
        m_body_cmd_f.set(pointer_actuation.mtr_body_cmd);
        t_body_cmd_f.set(pointer_actuation.rwa_body_cmd);
    }
}
