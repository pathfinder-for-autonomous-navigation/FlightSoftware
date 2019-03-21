#include "adcs_helpers.hpp"
#include <AttitudeEstimator.hpp>
#include <ADCSEnums.hpp>
#include "../../state/state_holder.hpp"
#include "../../state/device_states.hpp"

using Devices::adcs_system;
using State::ADCS::adcs_state_lock;

static unsigned char ssa_mode = SSAMode::IN_PROGRESS;
static unsigned int ssa_tries = 0; // Number of consecutive loops that we've tried to
                                   // collect SSA data
void ADCSControllers::read_adcs_data() {
    chMtxLock(&State::Hardware::adcs_device_lock);
        adcs_system().set_ssa_mode(ssa_mode);
    chMtxUnlock(&State::Hardware::adcs_device_lock);

    rwMtxRLock(&State::GNC::gnc_state_lock);
	for(int i = 0; i < 3; i++) ADCSControllers::Estimator::r_gps_ecef[i] = State::GNC::gps_position[i];
	for(int i = 0; i < 3; i++) ADCSControllers::Estimator::v_gps_ecef[i] = State::GNC::gps_velocity[i];
	for(int i = 0; i < 3; i++) ADCSControllers::Estimator::r2other_gps_ecef[i] = State::GNC::gps_position_other[i];
	for(int i = 0; i < 3; i++) ADCSControllers::Estimator::v2other_gps_ecef[i] = State::GNC::gps_velocity_other[i];
    for(int i = 0; i < 4; i++) ADCSControllers::Estimator::q_gps_ecef[i] = State::GNC::ecef_to_eci[i];
    ADCSControllers::Estimator::time = (unsigned int) State::GNC::get_current_time();
    rwMtxRUnlock(&State::GNC::gnc_state_lock);
    
    std::array<float, 3> rwa_speed_cmds_rd, rwa_ramps_rd, rwa_speeds_rd, gyro_data, mag_data;
    chMtxLock(&State::Hardware::adcs_device_lock);
        adcs_system().get_rwa(rwa_speed_cmds_rd.data(), ADCSControllers::Estimator::hwheel_sensor_body.data(), rwa_ramps_rd.data());
        adcs_system().get_imu(ADCSControllers::Estimator::rate_sensor_body.data(), ADCSControllers::Estimator::magfield_sensor_body.data());
    chMtxUnlock(&State::Hardware::adcs_device_lock);
    
    rwa_speeds_rd = ADCSControllers::Estimator::hwheel_sensor_body;
    gyro_data = ADCSControllers::Estimator::rate_sensor_body;
    mag_data = ADCSControllers::Estimator::magfield_sensor_body;
    State::write(State::ADCS::rwa_ramps_rd, rwa_ramps_rd, adcs_state_lock);
    State::write(State::ADCS::rwa_speed_cmds_rd, rwa_speed_cmds_rd, adcs_state_lock);
    State::write(State::ADCS::rwa_speeds_rd, rwa_speeds_rd, adcs_state_lock);
    State::write(State::ADCS::gyro_data, gyro_data, adcs_state_lock);
    State::write(State::ADCS::mag_data, mag_data, adcs_state_lock);

    // TODO insert position into estimator

    std::array<float, 3> ssa_vec;
    if (ssa_mode == SSAMode::IN_PROGRESS) {
        chMtxLock(&State::Hardware::adcs_device_lock);
            adcs_system().get_ssa(ssa_mode, ADCSControllers::Estimator::sun2sat_sensor_body.data());
        chMtxUnlock(&State::Hardware::adcs_device_lock);
        State::write(State::ADCS::ssa_vec, ADCSControllers::Estimator::sun2sat_sensor_body, adcs_state_lock);
        if (ssa_mode == SSAMode::COMPLETE) {
            rwMtxWLock(&adcs_state_lock);
            State::ADCS::ssa_vec = ssa_vec;
            State::ADCS::is_sun_vector_determination_working = true;
            State::ADCS::is_sun_vector_collection_working = true;
            rwMtxWUnlock(&adcs_state_lock);
            ssa_mode = SSAMode::IN_PROGRESS;
            ssa_tries = 0;
        }
        else if (ssa_mode == SSAMode::FAILURE) {
            rwMtxWLock(&adcs_state_lock);
            State::ADCS::is_sun_vector_determination_working = false;
            State::ADCS::is_sun_vector_collection_working = true;
            rwMtxWUnlock(&adcs_state_lock);
            // Don't worry; just keep trying, but let the ground know that SSA collection
            // is failing
            ssa_mode = SSAMode::IN_PROGRESS;
            ssa_tries = 0;
        }
        else if (ssa_tries < 5) {
            ssa_tries++;
        }
        else {
            // Tried too many times to collect SSA data and failed.
            State::ADCS::is_sun_vector_determination_working = false;
            State::ADCS::is_sun_vector_collection_working = false;
            // Don't worry; just keep trying, but let the ground know that SSA collection
            // is failing
            ssa_mode = SSAMode::IN_PROGRESS;
            ssa_tries = 0;
        }
    }
}