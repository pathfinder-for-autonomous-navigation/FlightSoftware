#include "AttitudeEstimator.hpp"

AttitudeEstimator::AttitudeEstimator(StateFieldRegistry &registry,
    unsigned int offset) 
    : TimedControlTask<void>(registry, offset),
    pan_epoch(),
    piksi_time_fp(find_readable_field<gps_time_t>("piksi.time", __FILE__, __LINE__)),
    pos_vec_ecef_fp(find_readable_field<d_vector_t>("piksi.pos", __FILE__, __LINE__)),
    ssa_vec_rd_fp(find_readable_field<f_vector_t>("adcs_box.sun_vec", __FILE__, __LINE__)),
    mag_vec_fp(find_readable_field<f_vector_t>("adcs_box.mag_vec", __FILE__, __LINE__)),
    data(),
    state(),
    estimate(),
    q_body_eci_sr(),
    q_body_eci_f("attitude_estimator.q_body_eci", q_body_eci_sr),
    w_body_sr(-55, 55, 32*3),
    w_body_f("attitude_estimator.w_body", w_body_sr)
    {

        //assert inputs are found
        assert(piksi_time_fp);
        assert(pos_vec_ecef_fp);
        assert(ssa_vec_rd_fp);
        assert(mag_vec_fp);

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
    data.t = (double)(unsigned long)(piksi_time_fp->get() - pan_epoch)/(1e9L);

    data.r_ecef = lin::Vector3d({
        pos_vec_ecef_fp->get()[0], 
        pos_vec_ecef_fp->get()[1], 
        pos_vec_ecef_fp->get()[2]
    });
    
    data.b_body = lin::Vector3f({
        mag_vec_fp->get()[0],
        mag_vec_fp->get()[1],
        mag_vec_fp->get()[2]
    });

    data.s_body = lin::Vector3f({
        ssa_vec_rd_fp->get()[0],
        ssa_vec_rd_fp->get()[1],
        ssa_vec_rd_fp->get()[2],
    });

}

void AttitudeEstimator::set_estimate(){

    f_quat_t q_temp;
    q_temp[0] = estimate.q_body_eci.operator()(0);
    q_temp[1] = estimate.q_body_eci.operator()(1);
    q_temp[2] = estimate.q_body_eci.operator()(2);
    q_temp[3] = estimate.q_body_eci.operator()(3);
    q_body_eci_f.set(q_temp);

    f_vector_t w_temp;
    w_temp[0] = estimate.w_body.operator()(0);
    w_temp[1] = estimate.w_body.operator()(1);
    w_temp[2] = estimate.w_body.operator()(2);  
    w_body_f.set(w_temp);

}

    