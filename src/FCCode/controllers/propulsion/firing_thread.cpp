#include "propulsion_tasks.hpp"
#include "../constants.hpp"
#include <tensor.hpp>
#include <AttitudeEstimator.hpp>

namespace Constants {
namespace Propulsion {
    static pla::Vec3f NVECTOR_1; // TODO initializers
    static pla::Vec3f NVECTOR_2;
    static pla::Vec3f NVECTOR_3;
    static pla::Vec3f NVECTOR_4;
    std::map<unsigned char, const pla::Vec3f> NOZZLE_VECTORS = {
        {2, NVECTOR_1},
        {3, NVECTOR_2},
        {4, NVECTOR_3},
        {5, NVECTOR_4}
    };
}
}

using State::Hardware::spike_and_hold_lock;
using namespace Constants::Propulsion;
using Devices::spike_and_hold;

thread_t* PropulsionTasks::firing_thread;

THD_WORKING_AREA(PropulsionTasks::firing_thread_wa, 1024);

THD_FUNCTION(PropulsionTasks::firing_fn, args) {
    rwMtxRLock(&State::ADCS::adcs_state_lock);
        std::array<float, 4> q_body;
        for(int i = 0; i < 4; i++) q_body[i] = ADCSControllers::Estimator::q_filter_body[i];
    rwMtxRUnlock(&State::ADCS::adcs_state_lock);
    pla::Vec3f impulse_vector_body;
    rwMtxRLock(&State::Propulsion::propulsion_state_lock);
        vect_rot(State::Propulsion::firing_data.impulse_vector.data(), q_body.data(), impulse_vector_body.get_data());
    rwMtxRUnlock(&State::Propulsion::propulsion_state_lock);
    std::array<unsigned int, 6> valve_timings;
    valve_timings[0] = 0;
    valve_timings[1] = 0;
    
    // Find three nozzles closest to impulse vector
    unsigned int farthest_nozzle = 2;
    float max_angle = 0;
    for(int i = 2; i < 6; i++) {
        float dot_product = Constants::Propulsion::NOZZLE_VECTORS.at(0) * impulse_vector_body;
        float magnitude_product = Constants::Propulsion::NOZZLE_VECTORS.at(i).length() * impulse_vector_body.length();
        float angle = acos(dot_product / magnitude_product);
        if (angle > max_angle) farthest_nozzle = i;
    }

    pla::Mat3x3f nozzle_vector_matrix; // Matrix of vector nozzles
    int k = 0; // Nozzle counter
    for(int i = 2; i < 6; i++) {
        if (i == farthest_nozzle) continue;
        for (int j = 0; j < 3; j++) nozzle_vector_matrix[k][j] = Constants::Propulsion::NOZZLE_VECTORS.at(i)[j];
        k++;
    }
    pla::Vec3f firing_times = nozzle_vector_matrix.inverse() * impulse_vector_body;
    k = 0;
    for(int i = 2; i < 6; i++) {
        if (i == farthest_nozzle) continue;
        valve_timings[i] = firing_times[k] * 1000; // Convert from seconds to milliseconds
        if (valve_timings[i] > 1000) valve_timings[i] = 1000; // Saturate firing
        k++;
    }

    // Add to delta-v
    rwMtxWLock(&State::Propulsion::propulsion_state_lock);
        State::Propulsion::delta_v_available += 
            vect_mag(State::Propulsion::firing_data.impulse_vector.data()) / Constants::Master::SPACECRAFT_MASS;
    rwMtxWUnlock(&State::Propulsion::propulsion_state_lock);
    chSysLock();
        debug_println("Initiating firing.");
        spike_and_hold.execute_schedule(valve_timings);
        debug_println("Completed firing.");
    chSysUnlock();

    change_propulsion_state(State::Propulsion::PropulsionState::IDLE);
    chThdExit((msg_t) 0);
}