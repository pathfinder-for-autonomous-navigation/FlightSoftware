#include "../../state/state_holder.hpp"
#include "adcs_helpers.hpp"
#include <AttitudeMath.hpp>
#include <AttitudeEstimator.hpp>
#include <tensor.hpp>

// Performs TRIAD given an ideal vector for the z face so that power
// collection is maximized. Ideal vector is specified in body coordinates.
static void command_optimal_long_edge(const pla::Vec3f& r) {
    // Unit vector z--antenna direction of satellite
    std::array<float, 3> z = {0,0,1};
    std::array<float, 3> sun_vector;
    rwMtxRLock(&State::ADCS::adcs_state_lock);
        for(int i = 0; i < 3; i++) sun_vector[i] = ADCSControllers::Estimator::sun2sat_filter_body[i];
    rwMtxRUnlock(&State::ADCS::adcs_state_lock);

    // Get normals to the four long edges of the satellite
    std::array<float, 3> long_edge_vectors[4];
    long_edge_vectors[0] = {1.0f / sqrt(2), 1.0f / sqrt(2), 0}; // 1/sqrt(2) (x + y)
    long_edge_vectors[1] = {1.0f / sqrt(2), -1.0f / sqrt(2), 0}; // 1/sqrt(2) (x - y)
    long_edge_vectors[2] = {-1.0f / sqrt(2), 1.0f / sqrt(2), 0}; // 1/sqrt(2) (-x + y)
    long_edge_vectors[3] = {-1.0f / sqrt(2), -1.0f / sqrt(2), 0}; // 1/sqrt(2) (-x - y)

    // Get quaternions for the TRIAD-based rotations to these four long edges
    std::array<float, 4> long_edge_quats[4];
    std::array<float, 4> eci_to_body_quat;
    for(int i = 0; i < 4; i++)
        quat_from_triad(z.data(), long_edge_vectors[i].data(), r.get_data(), sun_vector.data(), long_edge_quats[i].data());
    rwMtxRLock(&State::ADCS::adcs_state_lock);
        for(int i = 0; i < 4; i++)
            eci_to_body_quat[i] = ADCSControllers::Estimator::q_filter_body[i];
    rwMtxRLock(&State::ADCS::adcs_state_lock);
    // Find minimum-rotation quaternion from the current quaternion
    std::array<float, 4> optimal_edge_quat;
    float optimal_angle = 360;
    for(int i = 0; i < 4; i++) {
        std::array<float, 4> eci_to_new_body_quat;
        quat_cross_mult(eci_to_body_quat.data(), long_edge_quats[i].data(), eci_to_new_body_quat.data());
        float rotation_angle = 2.0*acos(abs(eci_to_new_body_quat[0]*eci_to_body_quat[0]));
        if (rotation_angle < optimal_angle) {
            optimal_edge_quat = long_edge_quats[i];
        }
    }
    
    // Get final command quaternion and write to state
    std::array<float, 4> cmd_attitude;
    quat_cross_mult(eci_to_body_quat.data(), optimal_edge_quat.data(), cmd_attitude.data());
    State::write_state(State::ADCS::cmd_attitude, cmd_attitude, State::ADCS::adcs_state_lock);
}

void ADCSControllers::point_for_standby() {
    // Radial vector from center of Earth
    rwMtxRLock(&State::GNC::gnc_state_lock);
        pla::Vec3f gps_position;
        pla::Vec4f ecef_to_eci;
        pla::Vec4f eci_to_body;
        for(int i = 0; i < 3; i++) gps_position[i] = State::GNC::gps_position[i];
        for(int i = 0; i < 4; i++) ecef_to_eci[i] = State::GNC::ecef_to_eci[i];
        rwMtxRLock(&State::ADCS::adcs_state_lock);
        for(int i = 0; i < 4; i++) eci_to_body[i] = ADCSControllers::Estimator::q_filter_body[i];
        rwMtxRUnlock(&State::ADCS::adcs_state_lock);

        // Transform r from ECEF to ECI
        pla::Vec3f gps_position_eci;
        vect_rot(gps_position.get_data(), ecef_to_eci.get_data(), gps_position_eci.get_data());

        // Transform r from ECI to body
        pla::Vec3f gps_position_body;
        vect_rot(gps_position_eci.get_data(), eci_to_body.get_data(), gps_position_body.get_data());
    rwMtxRUnlock(&State::GNC::gnc_state_lock);
    
    command_optimal_long_edge(gps_position_body);
}

void ADCSControllers::point_for_close_approach() {
    // Vector to location of other satellite.
    // We want this to point in the same direction as -z, so we need 
    // to take the negative of this vector in order to have the 
    // long-edge optimizer function work properly.
    // Radial vector from center of Earth
    rwMtxRLock(&State::GNC::gnc_state_lock);
        pla::Vec3f gps_position;
        pla::Vec3f gps_position_other;
        pla::Vec4f ecef_to_eci;
        pla::Vec4f eci_to_body;
        for(int i = 0; i < 3; i++) gps_position[i] = State::GNC::gps_position[i];
        for(int i = 0; i < 3; i++) gps_position_other[i] = State::GNC::gps_position_other[i];
        for(int i = 0; i < 4; i++) ecef_to_eci[i] = State::GNC::ecef_to_eci[i];
        rwMtxRLock(&State::ADCS::adcs_state_lock);
        for(int i = 0; i < 4; i++) eci_to_body[i] = ADCSControllers::Estimator::q_filter_body[i];
        rwMtxRUnlock(&State::ADCS::adcs_state_lock);
        
        pla::Vec3f gps_position_relative = gps_position - gps_position_other; // Negative of vector _to_ other satellite
        // Transform r from ECEF to ECI
        pla::Vec3f gps_position_relative_eci;
        vect_rot(gps_position_relative.get_data(), ecef_to_eci.get_data(), gps_position_relative_eci.get_data());
        // Transform r from ECI to body
        pla::Vec3f gps_position_relative_body;
        vect_rot(gps_position_relative_eci.get_data(), eci_to_body.get_data(), gps_position_relative_body.get_data());
    rwMtxRUnlock(&State::GNC::gnc_state_lock);
    
    command_optimal_long_edge(gps_position_relative_body);
}