#include "ADCSBoxController.hpp"
#include "adcs_state_t.enum"

#include <adcs/constants.hpp>
#include <adcs/havt_devices.hpp>

ADCSBoxController::ADCSBoxController(StateFieldRegistry &registry, 
    uint32_t offset, Devices::ADCS &_adcs)
    : TimedControlTask<void>(registry, "adcs_controller", offset),
    adcs_system(_adcs)
    {
        //find command statefields
        adcs_state_fp = find_writable_field<uint8_t>("adcs.state");

        rwa_mode_fp = find_writable_field<uint8_t>("adcs_cmd.rwa_mode");
        rwa_speed_cmd_fp = find_writable_field<f_vector_t>("adcs_cmd.rwa_speed_cmd");
        rwa_torque_cmd_fp = find_writable_field<f_vector_t>("adcs_cmd.rwa_torque_cmd");
        rwa_speed_filter_fp = find_writable_field<float>("adcs_cmd.rwa_speed_filter");
        rwa_ramp_filter_fp = find_writable_field<float>("adcs_cmd.rwa_ramp_filter");

        mtr_mode_fp = find_writable_field<uint8_t>("adcs_cmd.mtr_mode");
        mtr_cmd_fp = find_writable_field<f_vector_t>("adcs_cmd.mtr_cmd");
        mtr_limit_fp = find_writable_field<float>("adcs_cmd.mtr_limit");

        ssa_voltage_filter_fp = find_writable_field<float>("adcs_cmd.ssa_voltage_filter");

        imu_mode_fp = find_writable_field<uint8_t>("adcs_cmd.imu_mode");
        imu_mag_filter_fp = find_writable_field<float>("adcs_cmd.imu_mag_filter");
        imu_gyr_filter_fp = find_writable_field<float>("adcs_cmd.imu_gyr_filter");
        imu_gyr_temp_filter_fp = find_writable_field<float>("adcs_cmd.imu_gyr_temp_filter");
        imu_gyr_temp_kp_fp = find_writable_field<float>("adcs_cmd.imu_gyr_temp_kp");
        imu_gyr_temp_ki_fp = find_writable_field<float>("adcs_cmd.imu_gyr_temp_ki");
        imu_gyr_temp_kd_fp = find_writable_field<float>("adcs_cmd.imu_gyr_temp_kd");
        imu_gyr_temp_desired_fp = find_writable_field<float>("adcs_cmd.imu_gyr_temp_desired");
    
        
        //fill vector of pointers to statefields for havt
        havt_cmd_reset_vector_fp.reserve(adcs::havt::Index::_LENGTH);
        char buffer[50];
        for(uint32_t idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++)
        {
            std::memset(buffer, 0, sizeof(buffer));
            sprintf(buffer,"adcs_cmd.havt_reset");
            sprintf(buffer + strlen(buffer), "%u", idx);
            havt_cmd_reset_vector_fp.emplace_back(find_writable_field<bool>(buffer));
        }
        havt_cmd_disable_vector_fp.reserve(adcs::havt::Index::_LENGTH);
        for(uint32_t idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++)
        {
            std::memset(buffer, 0, sizeof(buffer));
            sprintf(buffer,"adcs_cmd.havt_disable");
            sprintf(buffer + strlen(buffer), "%u", idx);
            havt_cmd_disable_vector_fp.emplace_back(find_writable_field<bool>(buffer));
        }
    }

void ADCSBoxController::execute(){
    // set to passive/disabled if in startup
    if(adcs_state_fp->get() == static_cast<uint8_t>(adcs_state_t::startup))
        adcs_system.set_mode(adcs::ADCSMode::ADCS_PASSIVE);
    else
        adcs_system.set_mode(adcs::ADCSMode::ADCS_ACTIVE);

    // dump all commands
    if(rwa_mode_fp->get() == adcs::RWAMode::RWA_SPEED_CTRL)
        adcs_system.set_rwa_mode(rwa_mode_fp->get(), rwa_speed_cmd_fp->get());
    else if(rwa_mode_fp->get() == adcs::RWAMode::RWA_ACCEL_CTRL)
        adcs_system.set_rwa_mode(rwa_mode_fp->get(), rwa_torque_cmd_fp->get());
    else if(rwa_mode_fp->get() == adcs::RWAMode::RWA_DISABLED){
        adcs_system.set_rwa_mode(rwa_mode_fp->get(), std::array<float, 3>{0,0,0});
    }

    adcs_system.set_rwa_speed_filter(rwa_speed_filter_fp->get());
    adcs_system.set_ramp_filter(rwa_ramp_filter_fp->get());

    adcs_system.set_mtr_mode(mtr_mode_fp->get());
    adcs_system.set_mtr_cmd(mtr_cmd_fp->get());
    adcs_system.set_mtr_limit(mtr_limit_fp->get());

    //if calculation is complete/fail set the mode to in_progress to begin a new calc
    if(ssa_mode_fp->get() != adcs::SSAMode::SSA_IN_PROGRESS)
        adcs_system.set_ssa_mode(adcs::SSAMode::SSA_IN_PROGRESS);
    
    adcs_system.set_ssa_voltage_filter(ssa_voltage_filter_fp->get());

    adcs_system.set_imu_mode(imu_mode_fp->get());
    adcs_system.set_imu_mag_filter(imu_mag_filter_fp->get());
    adcs_system.set_imu_gyr_filter(imu_gyr_filter_fp->get());
    adcs_system.set_imu_gyr_temp_filter(imu_gyr_temp_filter_fp->get());
    adcs_system.set_imu_gyr_temp_kp(imu_gyr_temp_kp_fp->get());
    adcs_system.set_imu_gyr_temp_ki(imu_gyr_temp_ki_fp->get());
    adcs_system.set_imu_gyr_temp_kd(imu_gyr_temp_kd_fp->get());
    adcs_system.set_imu_gyr_temp_desired(imu_gyr_temp_desired_fp->get());

    std::bitset<adcs::havt::max_devices> temp_cmd_table(0);
    for(uint32_t idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++)
    {
        temp_cmd_table.set(idx, havt_cmd_reset_vector_fp[idx]->get());
    }
    adcs_system.set_havt_reset(temp_cmd_table);
    for(uint32_t idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++)
    {
        temp_cmd_table.set(idx, havt_cmd_disable_vector_fp[idx]->get());
    }
    adcs_system.set_havt_disable(temp_cmd_table);
}