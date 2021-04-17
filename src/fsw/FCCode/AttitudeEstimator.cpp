#include "AttitudeEstimator.hpp"

#include <adcs/constants.hpp>

#include <gnc/attitude_estimator.hpp>
#include <gnc/constants.hpp>

#include <lin/core.hpp>
#include <lin/generators/constants.hpp>
#include <lin/math.hpp>
#include <lin/queries.hpp>

AttitudeEstimator::AttitudeEstimator(StateFieldRegistry &registry) 
    : TimedControlTask<void>(registry, "adcs_estimator"),
    time_fp(FIND_READABLE_FIELD(double, orbit.time)),
    pos_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit.pos)),
    mag1_vec_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.mag1_vec)),
    mag2_vec_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.mag2_vec)),
    ssa_vec_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.ssa_vec)),
    gyr_vec_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.gyr_vec)),
    mag_flag_f("attitude_estimator.mag_flag", Serializer<bool>()),
    b_body_f("attitude_estimator.b_body"),
    q_body_eci_est_f("attitude_estimator.q_body_eci", Serializer<lin::Vector4f>()),
    w_body_est_f("attitude_estimator.w_body", 
    Serializer<lin::Vector3f>(0, 15*gnc::constant::deg_to_rad, 10)),
    fro_P_est_f("attitude_estimator.fro_P", Serializer<float>(0.0, 0.1, 32)) //max res 32
    {
        //Writable fields
        add_writable_field(mag_flag_f);

        //Add outputs
        add_internal_field(b_body_f);
        add_readable_field(q_body_eci_est_f);
        add_readable_field(w_body_est_f);
        add_readable_field(fro_P_est_f);

        // Default magnetometer
        mag_flag_f.set(false);
        b_body_f.set(lin::nans<lin::Vector3f>());

        // Default the gnc buffer
        state = gnc::AttitudeEstimatorState();
        data = gnc::AttitudeEstimatorData();
        estimate = gnc::AttitudeEstimate();
    }

void AttitudeEstimator::execute(){
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
        gnc::attitude_estimator_update(state, data, estimate);

        // Copy out the valid estimate
        if (estimate.is_valid) {
            q_body_eci_est_f.set(estimate.q_body_eci);
            w_body_est_f.set((w_body - estimate.gyro_bias).eval());
            fro_P_est_f.set(lin::fro(estimate.P));
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

void AttitudeEstimator::_nan_estimate() {
    q_body_eci_est_f.set(lin::nans<lin::Vector4f>());
    w_body_est_f.set(lin::nans<lin::Vector3f>());
    fro_P_est_f.set(gnc::constant::nan_f);
}
