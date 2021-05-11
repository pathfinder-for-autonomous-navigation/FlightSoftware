#include "ADCSCommander.hpp"
#include "adcs_state_t.enum"
#include <lin.hpp>
#include <gnc/utilities.hpp>
#include <gnc/constants.hpp>
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
    mag1_mode_f("adcs_cmd.mag1_mode", Serializer<unsigned char>(1)),
    mag2_mode_f("adcs_cmd.mag2_mode", Serializer<unsigned char>(1)),
    imu_mag_filter_f("adcs_cmd.imu_mag_filter", filter_sr),
    imu_gyr_filter_f("adcs_cmd.imu_gyr_filter", filter_sr),
    imu_gyr_temp_filter_f("adcs_cmd.imu_gyr_temp_filter", filter_sr),
    imu_gyr_temp_pwm_f("adcs_cmd.imu_gyr_temp_pwm", Serializer<unsigned char>()),
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
    add_writable_field(mag1_mode_f);
    add_writable_field(mag2_mode_f);
    add_writable_field(imu_mag_filter_f);
    add_writable_field(imu_gyr_filter_f);
    add_writable_field(imu_gyr_temp_filter_f);
    add_writable_field(imu_gyr_temp_pwm_f);
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
    pointer_rwa_torque_cmd = find_writable_field<lin::Vector3f>("pointer.rwa_torque_cmd", __FILE__, __LINE__);
    pointer_mtr_cmd        = find_writable_field<lin::Vector3f>("pointer.mtr_cmd", __FILE__, __LINE__);

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
    mag1_mode_f.set(adcs::IMU_MAG_NORMAL);
    mag2_mode_f.set(adcs::IMU_MAG_NORMAL);
    imu_mag_filter_f.set(1);
    imu_gyr_filter_f.set(1);
    imu_gyr_temp_filter_f.set(1);
    imu_gyr_temp_pwm_f.set(255); // default 100% pwm
    imu_gyr_temp_desired_f.set(20); // 20 degrees C
}

void ADCSCommander::execute() {
    adcs_state_t state = static_cast<adcs_state_t>(adcs_state_fp->get());

    switch(state) {
        /** don't apply any commands
        ADCSBoxController automatically sets ADCS to adcs::ADCSMode::ADCS_PASSIVE
        When MissionManager is in safehold, adcs_state = startup (here)
        */
        case adcs_state_t::startup:
            //zeroing out these commands are critical for ptesting
            rwa_torque_cmd_f.set({0,0,0});
            mtr_cmd_f.set({0,0,0});
            break;

        /** We do nothing here since ptest/ground will write into the adcs_cmd fields
         */
        case adcs_state_t::manual:
            break;        

        /** Detumble is mag rods only 
         * limited is equivalent to detumble
        */
        case adcs_state_t::limited:
        case adcs_state_t::detumble:
            rwa_mode_f.set(adcs::RWAMode::RWA_DISABLED);
            rwa_torque_cmd_f.set({0,0,0});

            mtr_mode_f.set(adcs::MTRMode::MTR_ENABLED);
            mtr_cmd_f.set(lin_to_std(pointer_mtr_cmd->get()));
            break;

        /** Apply no torque to the satellite
         */
        case adcs_state_t::zero_torque:
            rwa_mode_f.set(adcs::RWAMode::RWA_ACCEL_CTRL);
            rwa_torque_cmd_f.set({0,0,0});

            mtr_mode_f.set(adcs::MTRMode::MTR_DISABLED);
            mtr_cmd_f.set({0,0,0});            
            break;

        /** Hard stop the satellite wheels        
         * Use whatever commands required by the magnetorquer
         */
        case adcs_state_t::zero_L:
            rwa_mode_f.set(adcs::RWAMode::RWA_DISABLED);
            rwa_speed_cmd_f.set({0,0,0});

            mtr_mode_f.set(adcs::MTRMode::MTR_ENABLED);
            mtr_cmd_f.set(lin_to_std(pointer_mtr_cmd->get()));
            break;

        /** In these cases we are in acceleration wheel control, and mag rods on 
         * We use the commands dictated by the attitude_controller/pointer
        */
        case adcs_state_t::point_manual:
        case adcs_state_t::point_standby:
        case adcs_state_t::point_docking:
            rwa_mode_f.set(adcs::RWAMode::RWA_ACCEL_CTRL);
            rwa_torque_cmd_f.set(lin_to_std(pointer_rwa_torque_cmd->get()));

            mtr_mode_f.set(adcs::MTRMode::MTR_ENABLED);
            mtr_cmd_f.set(lin_to_std(pointer_mtr_cmd->get()));
            break;

        default:
            printf(debug_severity::error, "ADCSstate not defined: %d\n", static_cast<unsigned char>(state));
            break;
    }
}