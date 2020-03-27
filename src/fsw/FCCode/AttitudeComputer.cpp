#include "AttitudeComputer.hpp"
#include "adcs_state_t.enum"
#include <lin.hpp>
#include <gnc_utilities.hpp>
#include <gnc_constants.hpp>
#include <cmath>

constexpr float nan_f = std::numeric_limits<float>::quiet_NaN();

AttitudeComputer::AttitudeComputer(StateFieldRegistry& registry, unsigned int offset) :
    TimedControlTask<void>(registry, "attitude_computer", offset),
    adcs_vec1_current_f("adcs.compute.vec1.current", Serializer<lin::Vector3f>(0, 1, 100)),
    adcs_vec1_desired_f("adcs.compute.vec1.desired", Serializer<lin::Vector3f>(0, 1, 100)),
    adcs_vec2_current_f("adcs.compute.vec2.current", Serializer<lin::Vector3f>(0, 1, 100)),
    adcs_vec2_desired_f("adcs.compute.vec2.desired", Serializer<lin::Vector3f>(0, 1, 100))
{
    add_writable_field(adcs_vec1_current_f);
    add_writable_field(adcs_vec1_desired_f);
    add_writable_field(adcs_vec2_current_f);
    add_writable_field(adcs_vec2_desired_f);

    adcs_state_fp = find_writable_field<unsigned char>("adcs.state", __FILE__, __LINE__);
    q_body_eci_fp = find_readable_field<lin::Vector4f>("attitude_estimator.q_body_eci", __FILE__, __LINE__);
    ssa_vec_fp = find_readable_field<lin::Vector3f>("adcs_monitor.ssa_vec", __FILE__, __LINE__);
    pos_fp = find_readable_field<lin::Vector3d>("orbit.pos", __FILE__, __LINE__);
    baseline_pos_fp = find_readable_field<lin::Vector3d>("orbit.baseline_pos", __FILE__, __LINE__);

    // Initialize outputs to NaN values
    adcs_vec1_current_f.set({nan_f, nan_f, nan_f});
    adcs_vec2_current_f.set({nan_f, nan_f, nan_f});
    adcs_vec1_desired_f.set({nan_f, nan_f, nan_f});
    adcs_vec2_desired_f.set({nan_f, nan_f, nan_f});
}

void AttitudeComputer::execute() {
    adcs_state_t adcs_state = static_cast<adcs_state_t>(adcs_state_fp->get());

    lin::Vector4f q_body_eci = q_body_eci_fp->get();

    lin::Vector3f r_hat_eci = pos_fp->get();
    const bool posdata_is_set = std::isfinite(r_hat_eci(0));

    lin::Vector3f r_hat_body;
    if (posdata_is_set) {
        r_hat_eci = r_hat_eci / lin::norm(r_hat_eci);
        gnc::utl::rotate_frame(q_body_eci, r_hat_eci, r_hat_body);
    }

    switch(adcs_state) {
        case adcs_state_t::point_standby: {
            const lin::Vector3f ssa_vec = ssa_vec_fp->get();

            if (!posdata_is_set) {
                // We don't have a GPS reading. Point in the direction of the sun
                // since that's the only vector we know reliably. It'll charge up
                // our battery and we'll eventually have GPS coverage.

                // Pick "closest" long edge to point towards the Sun
                const lin::Vector3f long_edges[4] {
                    lin::Vector3f({std::sqrt(2.0f)/2.0f, std::sqrt(2.0f)/2.0f, 0.0f}),
                    lin::Vector3f({std::sqrt(2.0f)/2.0f, -std::sqrt(2.0f)/2.0f, 0.0f}),
                    lin::Vector3f({-std::sqrt(2.0f)/2.0f, std::sqrt(2.0f)/2.0f, 0.0f}),
                    lin::Vector3f({-std::sqrt(2.0f)/2.0f, -std::sqrt(2.0f)/2.0f, 0.0f})
                };
                size_t long_edge_choice = 0;
                float max_projection = -2; // Projection ranges from -1 to 1
                for(size_t i = 0; i < 4; i++) {
                    const float projection = lin::dot(long_edges[i], ssa_vec);
                    if (projection >= max_projection) {
                        max_projection = projection;
                        long_edge_choice = i;
                    }
                }

                adcs_vec1_current_f.set(ssa_vec);
                adcs_vec1_desired_f.set(long_edges[long_edge_choice]);
                adcs_vec2_current_f.set({nan_f, nan_f, nan_f});
                adcs_vec2_desired_f.set({nan_f, nan_f, nan_f});
            }
            else {
                // We've got a GPS reading. Point in a direction that
                // maximizes comms and power.
                lin::Vector3f r_cross_ssa_body = lin::cross(r_hat_body, ssa_vec);

                if (lin::norm(r_cross_ssa_body) > alignment_threshold) {
                    r_cross_ssa_body = r_cross_ssa_body / lin::norm(r_cross_ssa_body);
                }
                else {
                    r_cross_ssa_body = lin::Vector3f({0,0,1}); // No secondary pointing goal.
                }

                adcs_vec1_current_f.set(r_hat_body);
                adcs_vec1_desired_f.set({1,0,0});
                adcs_vec2_current_f.set(r_cross_ssa_body);
                adcs_vec2_desired_f.set({0,0,1});
            }
        }
        break;
        case adcs_state_t::point_docking: {
            lin::Vector3f baseline_pos_eci = baseline_pos_fp->get();
            baseline_pos_eci = baseline_pos_eci / lin::norm(baseline_pos_eci);
            lin::Vector3f baseline_pos_body;
            gnc::utl::rotate_frame(q_body_eci, baseline_pos_eci, baseline_pos_body);

            adcs_vec1_current_f.set(r_hat_body);
            adcs_vec1_desired_f.set({1,0,0});
            adcs_vec2_current_f.set(baseline_pos_body);
            adcs_vec2_desired_f.set({0,0,1});
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

    // Set if we're in one- or two-pointing mode
    const lin::Vector3f vec1_current = adcs_vec1_current_f.get();
    const lin::Vector3f vec2_current = adcs_vec2_current_f.get();
    const lin::Vector3f vec1_desired = adcs_vec1_desired_f.get();
    const lin::Vector3f vec2_desired = adcs_vec2_desired_f.get();
    if (std::isnan(vec2_current(0)) || std::isnan(vec2_desired(0)) ||
        std::abs(lin::dot(vec1_current, vec2_current)) >= std::cos(10.0f * gnc::constant::pi / 180.0f) ||
        std::abs(lin::dot(vec1_desired, vec2_desired)) >= std::cos(10.0f * gnc::constant::pi / 180.0f))
    {
        adcs_vec2_desired_f.set({nan_f, nan_f, nan_f});
    }
}
