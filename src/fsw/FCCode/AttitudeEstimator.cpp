#include "AttitudeEstimator.hpp"
#include <gnc/constants.hpp>

const gps_time_t AttitudeEstimator::pan_epoch(gnc::constant::init_gps_week_number,
                                              gnc::constant::init_gps_time_of_week,
                                              gnc::constant::init_gps_nanoseconds);

AttitudeEstimator::AttitudeEstimator(StateFieldRegistry &registry,
    unsigned int offset) 
    : TimedControlTask<void>(registry, "adcs_estimator", offset),
    data(),
    state(),
    estimate(),
    q_body_eci_f("attitude_estimator.q_body_eci", Serializer<lin::Vector4f>()),
    w_body_f("attitude_estimator.w_body", Serializer<lin::Vector3f>(-55, 55, 32*3)),
    h_body_f("attitude_estimator.h_body")
    {
        piksi_time_fp = find_readable_field<gps_time_t>("piksi.time", __FILE__, __LINE__),
        pos_vec_ecef_fp = find_readable_field<d_vector_t>("piksi.pos", __FILE__, __LINE__),
        ssa_vec_rd_fp = find_readable_field<lin::Vector3f>("adcs_monitor.ssa_vec", __FILE__, __LINE__),
        mag1_vec_fp = find_readable_field<lin::Vector3f>("adcs_monitor.mag1_vec", __FILE__, __LINE__),
        mag2_vec_fp = find_readable_field<lin::Vector3f>("adcs_monitor.mag2_vec", __FILE__, __LINE__),

        //Add outputs
        add_readable_field(q_body_eci_f);
        add_readable_field(w_body_f);
        add_internal_field(h_body_f);
    }

void AttitudeEstimator::execute(){
    set_data();
    gnc::estimate_attitude(state, data, estimate);
    set_estimate();
}

void AttitudeEstimator::set_data(){
    data.t = ((unsigned long)(piksi_time_fp->get() - pan_epoch)) / 1.0e9;

    const d_vector_t r_ecef = pos_vec_ecef_fp->get();
    data.r_ecef = {r_ecef[0], r_ecef[1], r_ecef[2]};

    // const lin::Vector3f mag2_vec = mag2_vec_fp->get();

    // TODO: LOGIC TO DECIDE IF WE WANT MAG1 or MAG2 data
    data.b_body = mag1_vec_fp->get();

    data.s_body = ssa_vec_rd_fp->get();
}

void AttitudeEstimator::set_estimate(){
    q_body_eci_f.set({
        estimate.q_body_eci(0),
        estimate.q_body_eci(1),
        estimate.q_body_eci(2),
        estimate.q_body_eci(3)
    });

    w_body_f.set(estimate.w_body);

    lin::Vector3f result = gnc::constant::J_sat * estimate.w_body;
    h_body_f.set(result.eval());
}
