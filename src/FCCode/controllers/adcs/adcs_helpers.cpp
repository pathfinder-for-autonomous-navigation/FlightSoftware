#include "../../state/state_holder.hpp"
#include "adcs_helpers.hpp"
#include <AttitudeMath.hpp>
#include <AttitudeEstimator.hpp>
#include <tensor.hpp>

// Performs TRIAD given an ideal vector for the z face so that power
// collection is maximized.
static void command_optimal_long_edge(const pla::Vec3f& r) {
    // Unit vector z--antenna direction of satellite
    std::array<float, 3> z = {0,0,1};

    // Get normals to the four long edges of the satellite
    std::array<float, 3> long_edge_vectors[4];
    long_edge_vectors[0] = {1.0f / sqrt(2), 1.0f / sqrt(2), 0}; // 1/sqrt(2) (x + y)
    long_edge_vectors[1] = {1.0f / sqrt(2), -1.0f / sqrt(2), 0}; // 1/sqrt(2) (x - y)
    long_edge_vectors[2] = {-1.0f / sqrt(2), 1.0f / sqrt(2), 0}; // 1/sqrt(2) (-x + y)
    long_edge_vectors[3] = {-1.0f / sqrt(2), -1.0f / sqrt(2), 0}; // 1/sqrt(2) (-x - y)

    // Get quaternions for the TRIAD-based rotations to these four long edges
    std::array<float, 4> long_edge_quats[4];
    for(int i = 0; i < 4; i++)
        quat_from_triad(z.data(), long_edge_vectors[i].data(), r.get_data(),
            ADCSControllers::Estimator::sun2sat_filter_body, long_edge_quats[i].data());
    // Find minimum-rotation quaternion from the current quaternion
    std::array<float, 4> optimal_edge_quat;
    std::array<float, 4> id_quat = {1,0,0,0}; // Identity quaternion; corresponds to zero rotation.
    float min_angle = 360.0f;
    for(int i = 0; i < 4; i++) {
        float angle = 2.0*acos(abs(id_quat[0]*long_edge_quats[i][0]));
        if (angle < min_angle) {
            min_angle = angle;
            optimal_edge_quat = long_edge_quats[i];
        }
    }

    // Get final command quaternion and write to state
    rwMtxWLock(&State::ADCS::adcs_state_lock);
        quat_cross_mult(ADCSControllers::Estimator::q_filter_body, optimal_edge_quat.data(), State::ADCS::cmd_attitude.data());
    rwMtxWUnlock(&State::ADCS::adcs_state_lock);
}

void ADCSControllers::point_for_standby() {
    // Radial vector from center of Earth
    rwMtxRLock(&State::GNC::gnc_state_lock);
        pla::Vec3f r;
        for(int i = 0; i < 3; i++) r[i] = State::GNC::gps_position[i];
    rwMtxRUnlock(&State::GNC::gnc_state_lock);
    command_optimal_long_edge(r);
}

void ADCSControllers::point_for_close_approach() {
    // Vector to location of other satellite.
    // We want this to point in the same direction as -z, so we need 
    // to take the negative of this vector in order to have the 
    // long-edge optimizer function work properly.
    rwMtxRLock(&State::GNC::gnc_state_lock);
        pla::Vec3f r;
        for(int i = 0; i < 3; i++) r[i] = -State::GNC::gps_position_other[i];
    rwMtxRUnlock(&State::GNC::gnc_state_lock);
    command_optimal_long_edge(r);
}