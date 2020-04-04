#include "ADCSCommander.hpp"
#include "adcs_state_t.enum"
#include <lin.hpp>
#include <gnc_utilities.hpp>
#include <gnc_constants.hpp>
#include <cmath>

#include <adcs/constants.hpp>
#include <adcs/havt_devices.hpp>

ADCSCommander::ADCSCommander(StateFieldRegistry& registry, unsigned int offset) :
    TimedControlTask<void>(registry, "adcs_commander", offset),
    filter_sr(0,1,8),
    rwa_mode_f("adcs_cmd.rwa_mode", Serializer<unsigned char>(2)),
    rwa_speed_cmd_f("adcs_cmd.rwa_speed_cmd", Serializer<f_vector_t>(
        adcs::rwa::min_speed_command, adcs::rwa::max_speed_command, 16*3)),
    rwa_torque_cmd_f("adcs_cmd.rwa_torque_cmd", Serializer<f_vector_t>(
        adcs::rwa::min_torque, adcs::rwa::max_torque, 16*3)),
    rwa_speed_filter_f("adcs_cmd.rwa_speed_filter", filter_sr),
    rwa_ramp_filter_f("adcs_cmd.rwa_ramp_filter", filter_sr),
    mtr_mode_f("adcs_cmd.mtr_mode", Serializer<unsigned char>(2)),
    mtr_cmd_f("adcs_cmd.mtr_cmd", Serializer<f_vector_t>(
        adcs::mtr::min_moment, adcs::mtr::max_moment, 16*3)),
    mtr_limit_f("adcs_cmd.mtr_limit", Serializer<float>(
        adcs::mtr::min_moment, adcs::mtr::max_moment, 16)),
    ssa_voltage_filter_f("adcs_cmd.ssa_voltage_filter", filter_sr),
    imu_mode_f("adcs_cmd.imu_mode", Serializer<unsigned char>(4)),
    imu_mag_filter_f("adcs_cmd.imu_mag_filter", filter_sr),
    imu_gyr_filter_f("adcs_cmd.imu_gyr_filter", filter_sr),
    imu_gyr_temp_filter_f("adcs_cmd.imu_gyr_temp_filter", filter_sr),
    k_sr(std::numeric_limits<float>::min(), std::numeric_limits<float>::max(),16),
    imu_gyr_temp_kp_f("adcs_cmd.imu_gyr_temp_kp", k_sr),
    imu_gyr_temp_ki_f("adcs_cmd.imu_gyr_temp_ki", k_sr),
    imu_gyr_temp_kd_f("adcs_cmd.imu_gyr_temp_kd", k_sr),
    imu_gyr_temp_desired_f("adcs_cmd.imu_gyr_temp_desired", 
        Serializer<float>(adcs::imu::min_eq_temp, adcs::imu::max_eq_temp, 8))
{
    // For ADCS Controller
    add_writable_field(rwa_mode_f);
    add_writable_field(rwa_speed_cmd_f);
    add_writable_field(rwa_torque_cmd_f);
    add_writable_field(rwa_speed_filter_f);
    add_writable_field(rwa_ramp_filter_f);
    add_writable_field(mtr_mode_f);
    add_writable_field(mtr_cmd_f);
    add_writable_field(mtr_limit_f);
    add_writable_field(ssa_voltage_filter_f);
    add_writable_field(imu_mode_f);
    add_writable_field(imu_mag_filter_f);
    add_writable_field(imu_gyr_filter_f);
    add_writable_field(imu_gyr_temp_filter_f);
    add_writable_field(imu_gyr_temp_kp_f);
    add_writable_field(imu_gyr_temp_ki_f);
    add_writable_field(imu_gyr_temp_kd_f);
    add_writable_field(imu_gyr_temp_desired_f);

    // reserve memory
    havt_cmd_reset_vector_f.reserve(adcs::havt::Index::_LENGTH);
    havt_cmd_disable_vector_f.reserve(adcs::havt::Index::_LENGTH);

    // fill vector of statefields for cmd havt
    char buffer[50];
    for (unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++ )
    {
        std::memset(buffer, 0, sizeof(buffer));
        sprintf(buffer,"adcs_cmd.havt_reset");
        sprintf(buffer + strlen(buffer), "%u", idx);
        havt_cmd_reset_vector_f.emplace_back(buffer, bool_sr);
        add_writable_field(havt_cmd_reset_vector_f[idx]);
        havt_cmd_reset_vector_f[idx].set(false); // default commands to false (don't apply cmd)
    }
    for (unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++ )
    {
        std::memset(buffer, 0, sizeof(buffer));
        sprintf(buffer,"adcs_cmd.havt_disable");
        sprintf(buffer + strlen(buffer), "%u", idx);
        havt_cmd_disable_vector_f.emplace_back(buffer, bool_sr);
        add_writable_field(havt_cmd_disable_vector_f[idx]);
        havt_cmd_disable_vector_f[idx].set(false); // default commands to false (don't apply cmd)
    }

    // find adcs state
    adcs_state_fp = find_writable_field<unsigned char>("adcs.state", __FILE__, __LINE__);

    // find outputs from AttitudeComputer
    adcs_vec1_current_fp = find_writable_field<lin::Vector3f>("adcs.compute.vec1.current", __FILE__, __LINE__);
    adcs_vec1_desired_fp = find_writable_field<lin::Vector3f>("adcs.compute.vec1.desired", __FILE__, __LINE__);
    adcs_vec2_current_fp = find_writable_field<lin::Vector3f>("adcs.compute.vec2.current", __FILE__, __LINE__);
    adcs_vec2_desired_fp = find_writable_field<lin::Vector3f>("adcs.compute.vec2.desired", __FILE__, __LINE__);

    // defaults, TODO: DECIDE DEFAULTS
    rwa_mode_f.set(adcs::RWAMode::RWA_DISABLED);
    rwa_speed_cmd_f.set({0,0,0});
    rwa_torque_cmd_f.set({0,0,0});
    rwa_speed_filter_f.set(1);
    rwa_ramp_filter_f.set(1);
    mtr_mode_f.set(adcs::MTRMode::MTR_DISABLED);
    mtr_cmd_f.set({0,0,0});
    mtr_limit_f.set(adcs::mtr::max_moment);
    ssa_voltage_filter_f.set(1);
    imu_mode_f.set(adcs::IMUMode::MAG1);
    imu_mag_filter_f.set(1);
    imu_gyr_filter_f.set(1);
    imu_gyr_temp_filter_f.set(1);
    imu_gyr_temp_kp_f.set(1);
    imu_gyr_temp_ki_f.set(1);
    imu_gyr_temp_kd_f.set(1);
    imu_gyr_temp_desired_f.set(20); // 20 degrees C
}

void ADCSCommander::execute() {
    adcs_state_t state = static_cast<adcs_state_t>(adcs_state_fp->get());

    switch(state) {
        case adcs_state_t::startup:             dispatch_startup();            break;
        case adcs_state_t::limited:             dispatch_limited();            break;
        case adcs_state_t::zero_torque:         dispatch_zero_torque();        break;
        case adcs_state_t::zero_L:              dispatch_zero_L();             break;
        case adcs_state_t::detumble:            dispatch_detumble();           break;
        case adcs_state_t::point_manual:        dispatch_manual();             break;
        case adcs_state_t::point_standby:       dispatch_standby();            break;
        case adcs_state_t::point_docking:       dispatch_docking();            break;
        default:
            printf(debug_severity::error, "ADCSstate not defined: %d\n", static_cast<unsigned char>(state));
            break;
    }
}

void ADCSCommander::dispatch_startup(){
    // don't apply any commands
    // ADCSBoxController automatically sets ADCS to adcs::ADCSMode::ADCS_PASSIVE
    // When MissionManager is in safehold, adcs_state = startup (here)
}
void ADCSCommander::dispatch_limited(){
    // TODO: set to desired mag_moment for pointing strat

    rwa_mode_f.set(adcs::RWAMode::RWA_DISABLED);
    mtr_mode_f.set(adcs::MTRMode::MTR_ENABLED);
}
void ADCSCommander::dispatch_zero_torque(){
    // wheels -> constant speed
    // MTRs -> 0,0,0

    rwa_mode_f.set(adcs::RWAMode::RWA_ACCEL_CTRL);
    mtr_mode_f.set(adcs::MTRMode::MTR_DISABLED);
}
void ADCSCommander::dispatch_zero_L(){
    // TODO: Run calculations to reduce spacecraft L to 0;

    rwa_mode_f.set(adcs::RWAMode::RWA_SPEED_CTRL);
    // set speed to 0
    mtr_mode_f.set(adcs::MTRMode::MTR_ENABLED);
}
void ADCSCommander::dispatch_detumble(){
    // TODO: run calculations such that we detumble

    rwa_mode_f.set(adcs::RWAMode::RWA_DISABLED);
    mtr_mode_f.set(adcs::MTRMode::MTR_ENABLED);
}
void ADCSCommander::dispatch_manual(){
    // do no calculations, let ground commands decide adcs commands
    // modes and cmds will be set by ground
}
void ADCSCommander::dispatch_standby(){
    // TODO: RUN CALCS FOR 1 STRAT

    rwa_mode_f.set(adcs::RWAMode::RWA_ACCEL_CTRL);
    mtr_mode_f.set(adcs::MTRMode::MTR_ENABLED);
}
void ADCSCommander::dispatch_docking(){
    // TODO: RUN CALCS FOR 2 STRAT

    rwa_mode_f.set(adcs::RWAMode::RWA_ACCEL_CTRL);
    mtr_mode_f.set(adcs::MTRMode::MTR_ENABLED);
}
