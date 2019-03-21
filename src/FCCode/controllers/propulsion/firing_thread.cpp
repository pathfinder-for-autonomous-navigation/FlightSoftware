#include "propulsion_tasks.hpp"
#include "../constants.hpp"
#include <tensor.hpp>
#include <AttitudeEstimator.hpp>

namespace Constants {
namespace Propulsion {
    // TODO fix later with actual values
    static pla::Vec3f NVECTOR_1 = {1.0f/sqrtf(3), 1.0f/sqrtf(3), 1.0f/sqrtf(3)};
    static pla::Vec3f NVECTOR_2 = {1.0f/sqrtf(3), 1.0f/sqrtf(3), 1.0f/sqrtf(3)};
    static pla::Vec3f NVECTOR_3 = {1.0f/sqrtf(3), 1.0f/sqrtf(3), 1.0f/sqrtf(3)};
    static pla::Vec3f NVECTOR_4 = {1.0f/sqrtf(3), 1.0f/sqrtf(3), 1.0f/sqrtf(3)};
    std::map<unsigned char, const pla::Vec3f> NOZZLE_VECTORS = {
        {2, NVECTOR_1},
        {3, NVECTOR_2},
        {4, NVECTOR_3},
        {5, NVECTOR_4}
    };
}
}

using State::Hardware::spike_and_hold_device_lock;
using namespace Constants::Propulsion;
using Devices::spike_and_hold;

thread_t* PropulsionTasks::firing_thread;

THD_WORKING_AREA(PropulsionTasks::firing_thread_wa, 1024);

THD_FUNCTION(PropulsionTasks::firing_fn, args) {
    // Convert impulse vector from ECI into body frame
    rwMtxRLock(&State::ADCS::adcs_state_lock);
        std::array<float, 4> q_body;
        for(int i = 0; i < 4; i++) q_body[i] = ADCSControllers::Estimator::q_filter_body[i];
    rwMtxRUnlock(&State::ADCS::adcs_state_lock);
    pla::Vec3f impulse_vector_body;
    std::array<float, 3> impulse_vector_eci = State::read(State::Propulsion::firing_data.impulse_vector, 
                                                          State::Propulsion::propulsion_state_lock);
    vect_rot(impulse_vector_eci.data(), q_body.data(), impulse_vector_body.get_data());
    
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

    // Compute duration of firings on each nozzle
    std::array<unsigned int, 6> valve_timings;
    valve_timings[0] = 0;
    valve_timings[1] = 0;
    pla::Vec3f firing_times = nozzle_vector_matrix.inverse() * impulse_vector_body;
    k = 0;
    for(int i = 2; i < 6; i++) {
        if (i == farthest_nozzle) continue;
        valve_timings[i] = firing_times[k] * 1000; // Convert from seconds to milliseconds
        if (valve_timings[i] > 1000) valve_timings[i] = 1000; // Saturate firing
        k++;
    }
    
    // Full-system lock so that the timing of firings is not affected by any interrupts.
    chSysLock();
        debug_println("Initiating firing.");
        if (State::Hardware::check_is_functional(&spike_and_hold())) {
            chMtxLock(&spike_and_hold_device_lock);
                spike_and_hold().execute_schedule(valve_timings);
            chMtxUnlock(&spike_and_hold_device_lock);
            debug_println("Completed firing.");
        }
        else {
            debug_println("Could not complete firing because DCDC is off.");
        }
    chSysUnlock();

    // TODO: if in debug mode, log the firing occurrence out to the console somehow so that
    // the new orbital dynamics can be propagated

    State::write(State::Propulsion::propulsion_state, 
                 State::Propulsion::PropulsionState::IDLE, 
                 State::Propulsion::propulsion_state_lock);
    chThdExit((msg_t) 0);
}