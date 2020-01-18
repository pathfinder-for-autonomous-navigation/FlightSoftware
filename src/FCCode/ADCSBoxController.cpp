#include "ADCSBoxController.hpp"
#include "adcs_state_t.enum"

#include <adcs_constants.hpp>
#include <adcs_havt_devices.hpp>

ADCSBoxController::ADCSBoxController(StateFieldRegistry &registry, 
    unsigned int offset, Devices::ADCS &_adcs)
    : TimedControlTask<void>(registry, "adcs_controller", offset),
    adcs_system(_adcs)
    {
        //find command statefields
        adcs_state_fp = find_writable_field<unsigned char>("adcs.state", __FILE__, __LINE__);

        rwa_mode_fp = find_writable_field<unsigned char>("adcs_cmd.rwa_cmd", __FILE__, __LINE__);
        rwa_cmd_fp = find_writable_field<f_vector_t>("adcs_cmd.rwa_cmd", __FILE__, __LINE__);
        rwa_speed_filter_fp = find_writable_field<float>("adcs_cmd.rwa_speed_filter", __FILE__, __LINE__);
        rwa_ramp_filter_fp = find_writable_field<float>("adcs_cmd.rwa_ramp_filter", __FILE__, __LINE__);

        mtr_mode_fp = find_writable_field<unsigned char>("adcs_cmd.mtr_mode", __FILE__, __LINE__);
        mtr_cmd_fp = find_writable_field<f_vector_t>("adcs_cmd.mtr_cmd", __FILE__, __LINE__);
        mtr_limit_fp = find_writable_field<float>("adcs_cmd.mtr_limit", __FILE__, __LINE__);

        ssa_mode_fp = find_writable_field<unsigned char>("adcs_cmd.ssa_mode", __FILE__, __LINE__);
        ssa_voltage_filter_fp = find_writable_field<float>("adcs_cmd.ssa_voltage_filter", __FILE__, __LINE__);

        imu_mode_fp = find_writable_field<unsigned char>("adcs_cmd.imu_mode", __FILE__, __LINE__);
        imu_mag_filter_fp = find_writable_field<float>("adcs_cmd.imu_mag_filter", __FILE__, __LINE__);
        imu_gyr_filter_fp = find_writable_field<float>("adcs_cmd.imu_gyr_filter", __FILE__, __LINE__);
        imu_gyr_temp_filter_fp = find_writable_field<float>("adcs_cmd.imu_gyr_temp_filter", __FILE__, __LINE__);
        imu_gyr_temp_kp_fp = find_writable_field<float>("adcs_cmd.imu_temp_kp", __FILE__, __LINE__);
        imu_gyr_temp_ki_fp = find_writable_field<float>("adcs_cmd.imu_temp_ki", __FILE__, __LINE__);
        imu_gyr_temp_kd_fp = find_writable_field<float>("adcs_cmd.imu_temp_kd", __FILE__, __LINE__);
        imu_gyr_temp_desired_fp = find_writable_field<float>("adcs_cmd.imu_gyr_temp_desired", __FILE__, __LINE__);
    
        //fill vector of pointers to statefields for havt
        char buffer[50];
        for(unsigned int idx = adcs_havt::Index::IMU_GYR; idx < adcs_havt::Index::_LENGTH; idx++)
        {
            std::memset(buffer, 0, sizeof(buffer));
            sprintf(buffer,"adcs_cmd.havt_device");
            sprintf(buffer + strlen(buffer), "%u", idx);
            havt_cmd_table_vector_fp.push_back(find_writable_field<bool>(buffer, __FILE__, __LINE__));
        }
    }

void ADCSBoxController::execute(){
    // set to passive/disabled if in startup
    // consider changing to writeable state_field in adcs box monitor
    if(adcs_state_fp->get() == static_cast<unsigned char>(adcs_state_t::startup))
        adcs_system.set_mode(ADCSMode::ADCS_PASSIVE);
    else
        adcs_system.set_mode(ADCSMode::ADCS_ACTIVE);

    //dump all commands straight in, ADCS deals with mode    
    adcs_system.set_rwa_mode(rwa_mode_fp->get(), rwa_cmd_fp->get());
    adcs_system.set_rwa_speed_filter(rwa_speed_filter_fp->get());
    adcs_system.set_ramp_filter(rwa_ramp_filter_fp->get());

    adcs_system.set_mtr_mode(mtr_mode_fp->get());
    adcs_system.set_mtr_cmd(mtr_cmd_fp->get());
    adcs_system.set_mtr_limit(mtr_limit_fp->get());

    //if calculation is complete/fail set the mode to in_progress to begin a new calc
    if(ssa_mode_fp->get() != SSAMode::SSA_IN_PROGRESS)
        adcs_system.set_ssa_mode(SSAMode::SSA_IN_PROGRESS);
    
    adcs_system.set_ssa_voltage_filter(ssa_voltage_filter_fp->get());

    adcs_system.set_imu_mode(imu_mode_fp->get());
    adcs_system.set_imu_mag_filter(imu_mag_filter_fp->get());
    adcs_system.set_imu_gyr_filter(imu_gyr_filter_fp->get());
    adcs_system.set_imu_gyr_temp_filter(imu_gyr_temp_filter_fp->get());
    adcs_system.set_imu_gyr_temp_kp(imu_gyr_temp_kp_fp->get());
    adcs_system.set_imu_gyr_temp_ki(imu_gyr_temp_ki_fp->get());
    adcs_system.set_imu_gyr_temp_kd(imu_gyr_temp_kd_fp->get());
    adcs_system.set_imu_gyr_temp_desired(imu_gyr_temp_desired_fp->get());

    // apply havt cmd table
    std::bitset<havt::max_devices> temp_cmd_table(0);
    for(unsigned int idx = adcs_havt::Index::IMU_GYR; idx < adcs_havt::Index::_LENGTH; idx++)
    {
        temp_cmd_table.set(idx, havt_cmd_table_vector_fp[idx]->get());
    }
    adcs_system.set_havt(temp_cmd_table);
}