#include "AttitudeEstimator.hpp"
#include <gnc_constants.hpp>

const gps_time_t AttitudeEstimator::pan_epoch(gnc::constant::init_gps_week_number,
                                              gnc::constant::init_gps_time_of_week,
                                              gnc::constant::init_gps_nanoseconds);

AttitudeEstimator::AttitudeEstimator(StateFieldRegistry &registry,
    unsigned int offset) 
    : TimedControlTask<void>(registry, "adcs_estimator", offset),
    data(),
    state(),
    estimate(),
    q_body_eci_sr(),
    q_body_eci_f("attitude_estimator.q_body_eci", q_body_eci_sr),
    w_body_sr(-55, 55, 32*3),
    w_body_f("attitude_estimator.w_body", w_body_sr)
    {
        piksi_time_fp = find_readable_field<gps_time_t>("piksi.time", __FILE__, __LINE__),
        pos_vec_ecef_fp = find_readable_field<d_vector_t>("piksi.pos", __FILE__, __LINE__),
        ssa_vec_rd_fp = find_readable_field<f_vector_t>("adcs_monitor.ssa_vec", __FILE__, __LINE__),
        mag_vec_fp = find_readable_field<f_vector_t>("adcs_monitor.mag_vec", __FILE__, __LINE__),

        //Add outputs
        add_readable_field(q_body_eci_f);
        add_readable_field(w_body_f);
    }

void AttitudeEstimator::execute(){
    set_data();
    gnc::estimate_attitude(state, data, estimate);
    set_estimate();
}

void AttitudeEstimator::set_data(){
    data.t = (double)(((unsigned long)(piksi_time_fp->get() - pan_epoch))/(1e9L));

    const d_vector_t r_ecef = pos_vec_ecef_fp->get();
    data.r_ecef = {r_ecef[0], r_ecef[1], r_ecef[2]};
    
    const f_vector_t mag_vec = mag_vec_fp->get();
    data.b_body = {mag_vec[0], mag_vec[1], mag_vec[2]};

    const f_vector_t f_vec = ssa_vec_rd_fp->get();
    data.s_body = {f_vec[0], f_vec[1], f_vec[2]};
}

void AttitudeEstimator::set_estimate(){
    f_quat_t q_temp;
    q_temp[0] = estimate.q_body_eci(0);
    q_temp[1] = estimate.q_body_eci(1);
    q_temp[2] = estimate.q_body_eci(2);
    q_temp[3] = estimate.q_body_eci(3);
    q_body_eci_f.set(q_temp);

    f_vector_t w_temp;
    w_temp[0] = estimate.w_body(0);
    w_temp[1] = estimate.w_body(1);
    w_temp[2] = estimate.w_body(2);  
    w_body_f.set(w_temp);
}
