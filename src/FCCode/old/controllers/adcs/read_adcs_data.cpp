#include <ADCSEnums.hpp>
#include <AttitudeEstimator.hpp>
#include "../../state/device_states.hpp"
#include "../../state/state_holder.hpp"
#include "adcs_helpers.hpp"

using Devices::adcs_system;
using State::ADCS::adcs_state_lock;
using State::Hardware::adcs_device_lock;
using namespace ADCSControllers;

static unsigned char ssa_mode = SSAMode::IN_PROGRESS;
static unsigned int ssa_tries = 0;  // Number of consecutive loops that we've tried to
                                    // collect SSA data
void ADCSControllers::read_adcs_data() {
    chMtxLock(&adcs_device_lock);
    adcs_system->update_hat();  // TODO fix
    chMtxUnlock(&adcs_device_lock);
    // TODO add logic to stop read of ADCS data if HAT is fucked up.

    chMtxLock(&adcs_device_lock);
    adcs_system->set_ssa_mode(ssa_mode);
    chMtxUnlock(&adcs_device_lock);

    rwMtxWLock(&adcs_state_lock);
    rwMtxRLock(&State::GNC::gnc_state_lock);
    for (int i = 0; i < 3; i++) Estimator::r_gps_eci[i] = State::GNC::gps_position[i];
    for (int i = 0; i < 4; i++) Estimator::q_gps_ecef[i] = State::GNC::ecef_to_eci[i];
    Estimator::time = (unsigned int)State::GNC::get_current_time();
    rwMtxRUnlock(&State::GNC::gnc_state_lock);
    rwMtxWUnlock(&adcs_state_lock);

    std::array<float, 3> rwa_speed_cmds_rd, rwa_ramps_rd, rwa_speeds_rd, gyro_data, mag_data;
    chMtxLock(&adcs_device_lock);
    rwMtxWLock(&adcs_state_lock);
    adcs_system->get_rwa(rwa_speed_cmds_rd.data(), rwa_speeds_rd.data(), rwa_ramps_rd.data());
    adcs_system->get_imu(gyro_data.data(), mag_data.data());
    rwMtxWUnlock(&adcs_state_lock);
    chMtxUnlock(&adcs_device_lock);

    rwMtxWLock(&adcs_state_lock);
    Estimator::hwheel_sensor_body = rwa_speeds_rd;
    Estimator::rate_sensor_body = gyro_data;
    Estimator::magfield_sensor_body = mag_data;
    State::ADCS::rwa_ramps_rd = rwa_ramps_rd;
    State::ADCS::rwa_speed_cmds_rd = rwa_speed_cmds_rd;
    State::ADCS::rwa_speeds_rd = rwa_speeds_rd;
    State::ADCS::gyro_data = gyro_data;
    State::ADCS::mag_data = mag_data;
    rwMtxWUnlock(&adcs_state_lock);

    std::array<float, 3> ssa_vec;
    if (ssa_mode == SSAMode::IN_PROGRESS) {
        chMtxLock(&adcs_device_lock);
        adcs_system->get_ssa(ssa_mode, Estimator::sat2sun_sensor_body.data());
        chMtxUnlock(&adcs_device_lock);
        State::write(State::ADCS::ssa_vec, Estimator::sat2sun_sensor_body, adcs_state_lock);
        if (ssa_mode == SSAMode::COMPLETE) {
            rwMtxWLock(&adcs_state_lock);
            State::ADCS::ssa_vec = ssa_vec;
            State::ADCS::is_sun_vector_determination_working = true;
            State::ADCS::is_sun_vector_collection_working = true;
            rwMtxWUnlock(&adcs_state_lock);
            ssa_mode = SSAMode::IN_PROGRESS;
            ssa_tries = 0;
        } else if (ssa_mode == SSAMode::FAILURE) {
            rwMtxWLock(&adcs_state_lock);
            State::ADCS::is_sun_vector_determination_working = false;
            State::ADCS::is_sun_vector_collection_working = true;
            rwMtxWUnlock(&adcs_state_lock);
            // Don't worry; just keep trying, but let the ground know that SSA
            // collection
            // is failing
            ssa_mode = SSAMode::IN_PROGRESS;
            ssa_tries = 0;
        } else if (ssa_tries < 5) {
            ssa_tries++;
        } else {
            // Tried too many times to collect SSA data and failed.
            State::ADCS::is_sun_vector_determination_working = false;
            State::ADCS::is_sun_vector_collection_working = false;
            // Don't worry; just keep trying, but let the ground know that SSA
            // collection
            // is failing
            ssa_mode = SSAMode::IN_PROGRESS;
            ssa_tries = 0;
        }
    }
}