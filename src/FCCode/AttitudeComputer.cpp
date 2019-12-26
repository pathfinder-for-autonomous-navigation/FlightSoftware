#include "AttitudeComputer.hpp"
#include "adcs_state_t.enum"
#include <lin.hpp>
#include <gnc_utilities.hpp>

AttitudeComputer::AttitudeComputer(StateFieldRegistry& registry, unsigned int offset) :
    TimedControlTask<void>(registry, "attitude_computer", offset),
    adcs_vec1_current_f("adcs.control.vec1.current", Serializer<f_vector_t>(0, 1, 100)),
    adcs_vec1_desired_f("adcs.control.vec1.desired", Serializer<f_vector_t>(0, 1, 100)),
    adcs_vec2_current_f("adcs.control.vec2.current", Serializer<f_vector_t>(0, 1, 100)),
    adcs_vec2_desired_f("adcs.control.vec2.desired", Serializer<f_vector_t>(0, 1, 100))
{
    add_writable_field(adcs_vec1_current_f);
    add_writable_field(adcs_vec1_desired_f);
    add_writable_field(adcs_vec2_current_f);
    add_writable_field(adcs_vec2_desired_f);

    adcs_state_fp = find_writable_field<unsigned char>("adcs.state", __FILE__, __LINE__);
    q_body_eci_fp = find_readable_field<f_quat_t>("attitude_estimator.q_body_eci", __FILE__, __LINE__);
    ssa_vec_fp = find_readable_field<f_vector_t>("adcs_monitor.ssa_vec", __FILE__, __LINE__);
    pos_fp = find_readable_field<d_vector_t>("orbit.pos", __FILE__, __LINE__);
    baseline_pos_fp = find_readable_field<d_vector_t>("orbit.baseline_pos", __FILE__, __LINE__);
}

void AttitudeComputer::execute() {
    adcs_state_t adcs_state = static_cast<adcs_state_t>(adcs_state_fp->get());

    const f_quat_t q_body_eci = q_body_eci_fp->get();
    lin::Vector4f body_eci_quat = {q_body_eci[0], q_body_eci[1], q_body_eci[2], q_body_eci[3]};

    const d_vector_t pos_eci = pos_fp->get();
    const bool posdata_is_set = !isnan(pos_eci[0]);

    lin::Vector3f r_hat_eci = {
        static_cast<float>(pos_eci[0]),
        static_cast<float>(pos_eci[1]),
        static_cast<float>(pos_eci[2])
    };

    lin::Vector3f r_hat_body;
    f_vector_t r_hat_body_vec;
    if (posdata_is_set) {
        r_hat_eci = r_hat_eci / lin::norm(r_hat_eci);
        gnc::utl::rotate_frame(body_eci_quat, r_hat_eci, r_hat_body);
        r_hat_body_vec = {r_hat_body(0), r_hat_body(1), r_hat_body(2)};
    }

    switch(adcs_state) {
        case adcs_state_t::point_standby: {
            const f_vector_t ssa_vec = ssa_vec_fp->get();

            if (!posdata_is_set) {
                // We don't have a GPS reading. Point in the direction of the sun
                // since that's the only vector we know reliably. It'll charge up
                // our battery and we'll eventually have GPS coverage.
                constexpr float nan = std::numeric_limits<float>::quiet_NaN();

                adcs_vec1_current_f.set(ssa_vec);
                adcs_vec1_desired_f.set({1,0,0});
                adcs_vec2_current_f.set({nan, nan, nan});
                adcs_vec2_desired_f.set({nan, nan, nan});
            }
            else {
                // We've got a GPS reading. Point in a direction that
                // maximizes comms and power.
                const lin::Vector3f ssa_normalized = {ssa_vec[0], ssa_vec[1], ssa_vec[2]};
                lin::Vector3f r_cross_ssa = lin::cross(r_hat_body, ssa_normalized);
                r_cross_ssa = r_cross_ssa / lin::norm(r_cross_ssa);
                const f_vector_t r_cross_ssa_vec = {r_cross_ssa(0), r_cross_ssa(1), r_cross_ssa(2)};

                adcs_vec1_current_f.set(r_hat_body_vec);
                adcs_vec1_desired_f.set({1,0,0});
                adcs_vec2_current_f.set(r_cross_ssa_vec);
                adcs_vec2_desired_f.set({0,0,1});
            }
        }
        break;
        case adcs_state_t::point_docking: {
            const d_vector_t baseline_pos = baseline_pos_fp->get();
            lin::Vector3f sat2target_r_eci_hat = {
                static_cast<float>(baseline_pos[0]),
                static_cast<float>(baseline_pos[1]),
                static_cast<float>(baseline_pos[2])
            };
            sat2target_r_eci_hat = sat2target_r_eci_hat / lin::norm(sat2target_r_eci_hat);
            lin::Vector3f sat2target_r_body_hat;
            gnc::utl::rotate_frame(body_eci_quat, sat2target_r_eci_hat, sat2target_r_body_hat);
            const f_vector_t sat2target_r_body_hat_vec =
                {sat2target_r_body_hat(0), sat2target_r_body_hat(1), sat2target_r_body_hat(2)};

            adcs_vec1_current_f.set(r_hat_body_vec);
            adcs_vec1_desired_f.set({1,0,0});
            adcs_vec2_current_f.set(sat2target_r_body_hat_vec);
            adcs_vec2_desired_f.set({0,0,-1});
        }
        break;
        case adcs_state_t::limited: {
            // ???
        }
        break;
        default:
        // Do nothing; there's no pointing strategy active.
        break;
    }
}
