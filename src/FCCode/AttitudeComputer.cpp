#include "AttitudeComputer.hpp"
#include "adcs_state_t.enum"
#include <lin.hpp>
#include <gnc_utilities.hpp>
#include <cmath>

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

    const f_quat_t q_body_eci_arr = q_body_eci_fp->get();
    lin::Vector4f q_body_eci = {q_body_eci_arr[0], q_body_eci_arr[1], q_body_eci_arr[2], q_body_eci_arr[3]};

    const d_vector_t pos_eci_arr = pos_fp->get();
    const bool posdata_is_set = std::isfinite(pos_eci_arr[0]);

    lin::Vector3f r_hat_body;
    f_vector_t r_hat_body_arr;
    if (posdata_is_set) {
        lin::Vector3f r_hat_eci = {
            static_cast<float>(pos_eci_arr[0]),
            static_cast<float>(pos_eci_arr[1]),
            static_cast<float>(pos_eci_arr[2])
        };
        r_hat_eci = r_hat_eci / lin::norm(r_hat_eci);
        gnc::utl::rotate_frame(q_body_eci, r_hat_eci, r_hat_body);
        r_hat_body_arr = {r_hat_body(0), r_hat_body(1), r_hat_body(2)};
    }

    switch(adcs_state) {
        case adcs_state_t::point_standby: {
            const f_vector_t ssa_vec = ssa_vec_fp->get();

            if (!posdata_is_set) {
                // We don't have a GPS reading. Point in the direction of the sun
                // since that's the only vector we know reliably. It'll charge up
                // our battery and we'll eventually have GPS coverage.
                constexpr float nan = std::numeric_limits<float>::quiet_NaN();

                // Pick "closest" long edge to point towards the Sun
                const f_vector_t long_edges_arrs[4] = {
                    {sqrtf(2)/2, sqrtf(2)/2, 0},
                    {sqrtf(2)/2, -sqrtf(2)/2, 0},
                    {-sqrtf(2)/2, sqrtf(2)/2, 0},
                    {-sqrtf(2)/2, -sqrtf(2)/2, 0},
                };
                size_t long_edge_choice = 0;
                float min_projection = -2;
                for(size_t i = 0; i < 4; i++) {
                    lin::Vector3f long_edge = {
                        long_edges_arrs[i][0],
                        long_edges_arrs[i][1],
                        long_edges_arrs[i][2]
                    };
                    lin::Vector3f ssa_vector = {ssa_vec[0], ssa_vec[1], ssa_vec[2]};
                    const float projection = 1 / lin::dot(long_edge, ssa_vector);
                    if (projection < min_projection) {
                        min_projection = projection;
                        long_edge_choice = i;
                    }
                }

                adcs_vec1_current_f.set(ssa_vec);
                adcs_vec1_desired_f.set(long_edges_arrs[long_edge_choice]);
                adcs_vec2_current_f.set({nan, nan, nan});
                adcs_vec2_desired_f.set({nan, nan, nan});
            }
            else {
                // We've got a GPS reading. Point in a direction that
                // maximizes comms and power.
                const lin::Vector3f ssa_body = {ssa_vec[0], ssa_vec[1], ssa_vec[2]};
                lin::Vector3f r_cross_ssa_body = lin::cross(r_hat_body, ssa_body);
                r_cross_ssa_body = r_cross_ssa_body / lin::norm(r_cross_ssa_body);
                const f_vector_t r_cross_ssa_body_arr = {
                    r_cross_ssa_body(0), r_cross_ssa_body(1), r_cross_ssa_body(2)};

                adcs_vec1_current_f.set(r_hat_body_arr);
                adcs_vec1_desired_f.set({1,0,0});
                adcs_vec2_current_f.set(r_cross_ssa_body_arr);
                adcs_vec2_desired_f.set({0,0,1});
            }
        }
        break;
        case adcs_state_t::point_docking: {
            const d_vector_t baseline_pos_eci_arr = baseline_pos_fp->get();
            lin::Vector3f baseline_pos_eci = {
                static_cast<float>(baseline_pos_eci_arr[0]),
                static_cast<float>(baseline_pos_eci_arr[1]),
                static_cast<float>(baseline_pos_eci_arr[2])
            };
            baseline_pos_eci = baseline_pos_eci / lin::norm(baseline_pos_eci);
            lin::Vector3f baseline_pos_body;
            gnc::utl::rotate_frame(q_body_eci, baseline_pos_eci, baseline_pos_body);
            const f_vector_t baseline_pos_body_arr =
                {baseline_pos_body(0), baseline_pos_body(1), baseline_pos_body(2)};

            adcs_vec1_current_f.set(r_hat_body_arr);
            adcs_vec1_desired_f.set({1,0,0});
            adcs_vec2_current_f.set(baseline_pos_body_arr);
            adcs_vec2_desired_f.set({0,0,-1});
        }
        break;
        case adcs_state_t::limited: {
            // Undefined behavior, as of now, while the GNC side of this controller
            // is still being developed.
        }
        break;
        case adcs_state_t::point_manual: {
            // Allow ground to set the desired pointing vectors.
        }
        break;
        default:
        // Do nothing; there's no pointing strategy active.
        break;
    }
}
