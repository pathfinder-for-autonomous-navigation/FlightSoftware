#include "ADCSCommander.hpp"
#include "adcs_state_t.enum"
#include <lin.hpp>
#include <gnc_utilities.hpp>
#include <gnc_constants.hpp>
#include <cmath>

#include <adcs/constants.hpp>
#include <adcs/havt_devices.hpp>

constexpr float nan_f = std::numeric_limits<float>::quiet_NaN();

ADCSCommander::ADCSCommander(StateFieldRegistry& registry, unsigned int offset) :
    TimedControlTask<void>(registry, "attitude_computer", offset),
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
    imu_gyr_temp_kp_f("adcs_cmd.imu_temp_kp", k_sr),
    imu_gyr_temp_ki_f("adcs_cmd.imu_temp_ki", k_sr),
    imu_gyr_temp_kd_f("adcs_cmd.imu_temp_kd", k_sr),
    imu_gyr_temp_desired_f("adcs_cmd.imu_gyr_temp_desired", Serializer<float>(adcs::imu::min_eq_temp, adcs::imu::max_eq_temp, 8)),
    bool_sr(),
    havt_cmd_apply_f("adcs_cmd.havt_apply", bool_sr)
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
    havt_cmd_table_vector_f.reserve(adcs::havt::Index::_LENGTH);

    // fill vector of statefields for cmd havt
    char buffer[50];
    for (unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++ )
    {
        std::memset(buffer, 0, sizeof(buffer));
        sprintf(buffer,"adcs_cmd.havt_device");
        sprintf(buffer + strlen(buffer), "%u", idx);
        havt_cmd_table_vector_f.emplace_back(buffer, bool_sr);
        add_writable_field(havt_cmd_table_vector_f[idx]);

        // havt_cmd_table_vector_f[idx].set(true);
    }

    havt_read_table_vector_fp.reserve(adcs::havt::Index::_LENGTH);
    //fill vector of pointers to statefields of adcs_monitor.havt_device
    for (unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++ )
    {
        std::memset(buffer, 0, sizeof(buffer));
        sprintf(buffer,"adcs_monitor.havt_device");
        sprintf(buffer + strlen(buffer), "%u", idx);
        havt_read_table_vector_fp.emplace_back(find_readable_field<bool>(buffer, __FILE__, __LINE__));
    }

    // find adcs state
    adcs_state_fp = find_writable_field<unsigned char>("adcs.state", __FILE__, __LINE__);

    add_writable_field(havt_cmd_apply_f);
    // default initialize to false
    havt_cmd_apply_f.set(false);
}

void ADCSCommander::execute() {
    
    // controller automatically restarts ssa vector calculation, 
    // no need to restart

    adcs_state_t state = static_cast<adcs_state_t>(adcs_state_fp->get());
    // default
    switch(state) {
        // apply cmds that do nothing
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

}
void ADCSCommander::dispatch_limited(){

}
void ADCSCommander::dispatch_zero_torque(){

}
void ADCSCommander::dispatch_zero_L(){

}
void ADCSCommander::dispatch_detumble(){

}
void ADCSCommander::dispatch_manual(){

}
void ADCSCommander::dispatch_standby(){

}
void ADCSCommander::dispatch_docking(){
    
}
