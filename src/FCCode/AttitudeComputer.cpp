#include "AttitudeComputer.hpp"
#include "adcs_state_t.enum"
#include <lin.hpp>

AttitudeComputer::AttitudeComputer(StateFieldRegistry& registry, unsigned int offset) :
    TimedControlTask<void>(registry, "attitude_computer", offset),
    adcs_state_f("adcs.state", Serializer<unsigned char>(8)),
    adcs_vec1_current_f("adcs.control.vec1.current", Serializer<f_vector_t>(0, 1, 100)),
    adcs_vec1_desired_f("adcs.control.vec1.desired", Serializer<f_vector_t>(0, 1, 100)),
    adcs_vec2_current_f("adcs.control.vec2.current", Serializer<f_vector_t>(0, 1, 100)),
    adcs_vec2_desired_f("adcs.control.vec2.desired", Serializer<f_vector_t>(0, 1, 100))
{
    add_writable_field(const_cast<WritableStateField<unsigned char>&>(adcs_state_f));
    add_writable_field(adcs_vec1_current_f);
    add_writable_field(adcs_vec1_desired_f);
    add_writable_field(adcs_vec2_current_f);
    add_writable_field(adcs_vec2_desired_f);

    ssa_vec_fp = find_readable_field<f_vector_t>("adcs_monitor.ssa_vec", __FILE__, __LINE__);

    time_fp = find_readable_field<gps_time_t>("piksi.time", __FILE__, __LINE__);
    pos_fp = find_readable_field<d_vector_t>("piksi.pos", __FILE__, __LINE__);
    baseline_pos_fp = find_readable_field<d_vector_t>("piksi.baseline_pos", __FILE__, __LINE__);
}

void AttitudeComputer::execute() {
    adcs_state_t adcs_state = static_cast<adcs_state_t>(adcs_state_f.get());

    const d_vector_t pos = pos_fp->get();
    lin::Vector3f pos_normalized = {(float) pos[0], (float) pos[1], (float) pos[2]};
    pos_normalized = pos_normalized / lin::norm(pos_normalized);
    const f_vector_t pos_normalized_vec = {pos_normalized(0), pos_normalized(1), pos_normalized(2)};

    const d_vector_t baseline_pos = baseline_pos_fp->get();
    lin::Vector3f baseline_pos_normalized =
        {(float) baseline_pos[0], (float) baseline_pos[1], (float) baseline_pos[2]};
    baseline_pos_normalized = baseline_pos_normalized / lin::norm(baseline_pos_normalized);
    const f_vector_t baseline_pos_normalized_vec =
        {baseline_pos_normalized(0), baseline_pos_normalized(1), baseline_pos_normalized(2)};
    
    const f_vector_t ssa_vec = ssa_vec_fp->get();
    lin::Vector3f ssa_normalized = {ssa_vec[0], ssa_vec[1], ssa_vec[2]};
    lin::Vector3f r_cross_ssa = lin::cross(pos_normalized, ssa_normalized);
    r_cross_ssa = r_cross_ssa / lin::norm(r_cross_ssa);
    const f_vector_t r_cross_ssa_vec = {r_cross_ssa(0), r_cross_ssa(1), r_cross_ssa(2)};

    switch(adcs_state) {
        case adcs_state_t::point_standby: {
            if (time_fp->get().is_set) {
                // We've got a GPS reading, point in a direction that
                // maximizes power.
                adcs_vec1_current_f.set(pos_normalized_vec);
                adcs_vec1_desired_f.set({1,0,0});
                adcs_vec2_current_f.set(r_cross_ssa_vec);
                adcs_vec2_desired_f.set({0,0,1});
            }
            else {
                // We don't have a GPS reading. Point in a direction that
                // maximizes the chances of getting one.
                constexpr float nan = std::numeric_limits<float>::quiet_NaN();

                adcs_vec1_current_f.set(ssa_vec);
                adcs_vec1_desired_f.set({1,0,0});
                adcs_vec2_current_f.set({nan, nan, nan});
                adcs_vec2_desired_f.set({nan, nan, nan});
            }
        }
        break;
        case adcs_state_t::point_docking: {
            adcs_vec1_current_f.set(pos_normalized_vec);
            adcs_vec1_desired_f.set({1,0,0});
            adcs_vec2_current_f.set(baseline_pos_normalized_vec);
            adcs_vec2_desired_f.set({0,0,-1});
        }
        break;
        case adcs_state_t::limited: {
            // ???
        }
        break;
        default:
        // Do nothing; there's no pointing mode active.
        break;
    }
}
