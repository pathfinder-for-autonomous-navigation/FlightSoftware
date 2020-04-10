#include "ADCSBoxController.hpp"
#include "adcs_state_t.enum"

#include <adcs/constants.hpp>
#include <adcs/havt_devices.hpp>

ADCSBoxController::ADCSBoxController(StateFieldRegistry &registry, 
    unsigned int offset, Devices::ADCS &_adcs)
    : TimedControlTask<void>(registry, "adcs_controller", offset),
    adcs_system(_adcs)
    {
        //find command statefields
        adcs_state_fp = find_writable_field<unsigned char>("adcs.state", __FILE__, __LINE__);

        rwa_mode_fp = find_writable_field<unsigned char>("adcs_cmd.rwa_mode", __FILE__, __LINE__);
        rwa_speed_cmd_fp = find_writable_field<f_vector_t>("adcs_cmd.rwa_speed_cmd", __FILE__, __LINE__);
        rwa_torque_cmd_fp = find_writable_field<f_vector_t>("adcs_cmd.rwa_torque_cmd", __FILE__, __LINE__);
        rwa_speed_filter_fp = find_writable_field<float>("adcs_cmd.rwa_speed_filter", __FILE__, __LINE__);
        rwa_ramp_filter_fp = find_writable_field<float>("adcs_cmd.rwa_ramp_filter", __FILE__, __LINE__);

        mtr_mode_fp = find_writable_field<unsigned char>("adcs_cmd.mtr_mode", __FILE__, __LINE__);
        mtr_cmd_fp = find_writable_field<f_vector_t>("adcs_cmd.mtr_cmd", __FILE__, __LINE__);
        mtr_limit_fp = find_writable_field<float>("adcs_cmd.mtr_limit", __FILE__, __LINE__);

        ssa_mode_fp = find_readable_field<int>("adcs_monitor.ssa_mode", __FILE__, __LINE__);
        ssa_voltage_filter_fp = find_writable_field<float>("adcs_cmd.ssa_voltage_filter", __FILE__, __LINE__);

        imu_mode_fp = find_writable_field<unsigned char>("adcs_cmd.imu_mode", __FILE__, __LINE__);
        imu_mag_filter_fp = find_writable_field<float>("adcs_cmd.imu_mag_filter", __FILE__, __LINE__);
        imu_gyr_filter_fp = find_writable_field<float>("adcs_cmd.imu_gyr_filter", __FILE__, __LINE__);
        imu_gyr_temp_filter_fp = find_writable_field<float>("adcs_cmd.imu_gyr_temp_filter", __FILE__, __LINE__);
        imu_gyr_temp_kp_fp = find_writable_field<float>("adcs_cmd.imu_gyr_temp_kp", __FILE__, __LINE__);
        imu_gyr_temp_ki_fp = find_writable_field<float>("adcs_cmd.imu_gyr_temp_ki", __FILE__, __LINE__);
        imu_gyr_temp_kd_fp = find_writable_field<float>("adcs_cmd.imu_gyr_temp_kd", __FILE__, __LINE__);
        imu_gyr_temp_desired_fp = find_writable_field<float>("adcs_cmd.imu_gyr_temp_desired", __FILE__, __LINE__);
    
        
        //fill vector of pointers to statefields for havt
        havt_cmd_reset_vector_fp.reserve(adcs::havt::Index::_LENGTH);
        char buffer[50];
        for(unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++)
        {
            std::memset(buffer, 0, sizeof(buffer));
            sprintf(buffer,"adcs_cmd.havt_reset");
            sprintf(buffer + strlen(buffer), "%u", idx);
            havt_cmd_reset_vector_fp.emplace_back(find_writable_field<bool>(buffer, __FILE__, __LINE__));
        }
        havt_cmd_disable_vector_fp.reserve(adcs::havt::Index::_LENGTH);
        for(unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++)
        {
            std::memset(buffer, 0, sizeof(buffer));
            sprintf(buffer,"adcs_cmd.havt_disable");
            sprintf(buffer + strlen(buffer), "%u", idx);
            havt_cmd_disable_vector_fp.emplace_back(find_writable_field<bool>(buffer, __FILE__, __LINE__));
        }
    }

void ADCSBoxController::execute(){
    // set to passive/disabled if in startup
    if(adcs_state_fp->get() == static_cast<unsigned char>(adcs_state_t::startup))
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

    // send_cmd_table is true iff there is a non zero bit in the reset_vector or disable_vector
    bool send_cmd_table = false;
    for(unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++)
    {
        // the bit is high if there is a command to reset the device at [idx]
        bool reset_get = havt_cmd_reset_vector_fp[idx]->get();
        if(reset_get) {
            temp_cmd_table.set(idx, reset_get);
            send_cmd_table = true;

            // clear the state field now that it's loaded into temp_cmd_table
            const_cast<WritableStateField<bool>*>(havt_cmd_reset_vector_fp[idx])->set(false);
        }
    }

    // dispatch the i2c call to send the reset table 
    // iff there was a non-zero bit in the reset table
    if(send_cmd_table)
        adcs_system.set_havt_reset(temp_cmd_table);

    send_cmd_table = false;
    for(unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++)
    {
        // the bit is high if there is a command to disable the device at [idx]
        bool disable_get = havt_cmd_disable_vector_fp[idx]->get();
        if(disable_get){
            temp_cmd_table.set(idx, disable_get);
            send_cmd_table = true;

            // clear the state field now that it's loaded into temp_cmd_table
            const_cast<WritableStateField<bool>*>(havt_cmd_disable_vector_fp[idx])->set(false);
        }
    }
    
    // dispatch the i2c call to send the disable table 
    // iff there was a non-zero bit in the disable table
    if(send_cmd_table)
        adcs_system.set_havt_disable(temp_cmd_table);
}