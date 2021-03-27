#include "AttitudeEstimator.hpp"

#include <adcs/constants.hpp>

#include <gnc/attitude_estimator.hpp>
#include <gnc/constants.hpp>

#include <lin/core.hpp>
#include <lin/generators.hpp>
#include <lin/math.hpp>
#include <lin/queries.hpp>
#include <lin/references.hpp>

AttitudeEstimator::AttitudeEstimator(StateFieldRegistry &registry) 
    : ControlTask<void>(registry),
      time_valid_fp(FIND_READABLE_FIELD(bool, time.valid)),
      time_s_fp(FIND_READABLE_FIELD(double, time.s)),
      orbit_valid_fp(FIND_READABLE_FIELD(bool, orbit.valid)),
      orbit_pos_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit.pos)),
      adcs_gyr_functional_fp(FIND_READABLE_FIELD(bool, adcs_monitor.havt_device0)),
      adcs_gyr_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.gyr_vec)),
      adcs_mag1_functional_fp(FIND_READABLE_FIELD(bool, adcs_monitor.havt_device1)),
      adcs_mag1_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.mag1_vec)),
      adcs_mag2_functional_fp(FIND_READABLE_FIELD(bool, adcs_monitor.havt_device2)),
      adcs_mag2_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.mag2_vec)),
      adcs_ssa_mode_fp(FIND_READABLE_FIELD(unsigned char, adcs_monitor.ssa_mode)),
      adcs_ssa_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.ssa_vec)),
      attitude_estimator_b_valid_f("attitude_estimator.b_valid"),
      attitude_estimator_b_body_f("attitude_estimator.b_body"),
      attitude_estimator_valid_f("attitude_estimator.valid", Serializer<bool>()),
      attitude_estimator_q_body_eci_f("attitude_estimator.q_body_eci", Serializer<lin::Vector4f>()),
      attitude_estimator_p_body_eci_sigma_f("attitude_estimator.p_body_eci_sigma_f", Serializer<lin::Vector3f>(/* TODO */)),
      attitude_estimator_w_body_f("attitude_estimator.w_body"),
      attitude_estimator_w_bias_body_f("attitude_estimator.w_bias_body", Serializer<lin::Vector3f>(/* TODO */)),
      attitude_estimator_w_bias_body_sigma_f("attitude_estimator.w_bias_sigma_body", Serializer<lin::Vector3f>(/* TODO */)),
      attitude_estimator_L_body_f("attitude_estimator.L_body", Serializer<lin::Vector3f>(/* TODO */)),
      attitude_estimator_reset_cmd_f("attitude_estimator.reset_cmd", Serializer<bool>()),
      attitude_estimator_mag_flag_f("attitude_estimator.mag_flag", Serializer<bool>())
{
    add_internal_field(attitude_estimator_b_valid_f);
    add_internal_field(attitude_estimator_b_body_f);
    add_readable_field(attitude_estimator_valid_f);
    add_readable_field(attitude_estimator_q_body_eci_f);
    add_readable_field(attitude_estimator_p_body_eci_sigma_f);
    add_internal_field(attitude_estimator_w_body_f);
    add_readable_field(attitude_estimator_w_bias_body_f);
    add_readable_field(attitude_estimator_w_bias_body_sigma_f);
    add_readable_field(attitude_estimator_L_body_f);
    add_writable_field(attitude_estimator_reset_cmd_f);
    add_writable_field(attitude_estimator_mag_flag_f);

    attitude_estimator_valid_f.set(false);
    attitude_estimator_b_body_f.set(lin::zeros<lin::Vector3f>());
    attitude_estimator_q_body_eci_f.set(lin::zeros<lin::Vector4f>());
    attitude_estimator_p_body_eci_sigma_f.set(lin::zeros<lin::Vector3f>());
    attitude_estimator_w_body_f.set(lin::zeros<lin::Vector3f>());
    attitude_estimator_w_bias_body_f.set(lin::zeros<lin::Vector3f>());
    attitude_estimator_w_bias_body_sigma_f.set(lin::zeros<lin::Vector3f>());
    attitude_estimator_L_body_f.set(lin::zeros<lin::Vector3f>());
    attitude_estimator_reset_cmd_f.set(false);
    attitude_estimator_mag_flag_f.set(false);

    _state = gnc::AttitudeEstimatorState();
    _data = gnc::AttitudeEstimatorData();
    _estimate = gnc::AttitudeEstimate();
}

void AttitudeEstimator::execute()
{
    /* Handle the processing of magnetometer information.
     */
    auto const have_functional_magnetometer =
            !adcs_mag1_functional_fp->get() && !adcs_mag2_functional_fp->get();

    if (have_functional_magnetometer)
    {
        
    }

    /* Reset the attitude estimator if the time estimate is invalid, orbit
     * estimate is invalid, or we don't have enough data from the ADCS box to
     * update the estimator.
     */
    {
        auto const should_reset = !time_valid_fp->get() ||
                !orbit_valid_fp->get() || !adcs_gyr_functional_fp->get() ||
                (!adcs_mag1_functional_fp->get() && !adcs_mag2_functional_fp->get());

        if (should_reset)
        {
            attitude_estimator_valid_f.set(false);
            attitude_estimator_reset_cmd_f.set(false);

            _state = gnc::AttitudeEstimatorState();
            _estimate = gnc::AttitudeEstimate();
        }
    }

    /* Process magnetometer data.
     *
     * If the magnetometer flag is set to false, we'll listen to the first
     * magnetomer and the second magnetometer otherwise.
     */
    auto const b_body = [&]() -> lin::Vector3f
    {
        auto const use_mag1 =
            attitude_estimator_mag_flag_f.get() && adcs_mag1_functional_fp->get();

        return use_mag1 ? adcs_mag1_fp->get() : adcs_mag2_fp->get();
    }();
    attitude_estimator_b_body_f.set(b_body);

    auto const time_s = time_s_fp->get();
    auto const orbit_pos = orbit_pos_fp->get();
    auto const adcs_gyr = adcs_gyr_fp->get();
    auto const adcs_ssa = adcs_ssa_fp->get();


    // Copy in all of our inputs
    double t             = time_fp->get();
    lin::Vector3d r_ecef = pos_fp->get();
    lin::Vector3f s_body = ssa_vec_fp->get();
    lin::Vector3f w_body = gyr_vec_fp->get();

    // Normalize
    s_body = s_body / lin::norm(s_body);

    // Handle the special magnetometer case
    lin::Vector3f b_body = mag_flag_f.get() ? mag2_vec_fp->get() : mag1_vec_fp->get(); // TODO : Choose default mag
    if (!lin::all(lin::isfinite(b_body)))
        b_body = !mag_flag_f.get() ? mag2_vec_fp->get() : mag1_vec_fp->get();

    b_body_f.set(b_body);

    // The filter is already up and running
    if (state.is_valid) {
        // Populate the input struct
        data = gnc::AttitudeEstimatorData();
        data.t = t;
        data.r_ecef = r_ecef;
        data.b_body = b_body;
        data.s_body = s_body;
        data.w_body = w_body;

        // Update the filter
        gnc::attitude_estimator_update(_state, _data, _estimate);

        // Copy out the valid estimate
        if (estimate.is_valid) {
            q_body_eci_est_f.set(_estimate.q_body_eci);
            w_body_est_f.set((_w_body - _estimate.gyro_bias).eval());
            fro_P_est_f.set(lin::fro(_estimate.P));
        }
        // Handle an invalid estimate
        else {
            _nan_estimate();
        }
    }
    // The filter needs to be initialized
    else {
        // All we can give is a NaN estimate and hope the reset works on the next
        // control cycle
        _nan_estimate();

        gnc::attitude_estimator_reset(state, t, r_ecef, b_body, s_body);
    }
}
