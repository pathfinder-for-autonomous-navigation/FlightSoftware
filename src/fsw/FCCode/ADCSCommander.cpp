#include "ADCSCommander.hpp"
#include "adcs_state_t.enum"
#include <lin.hpp>
#include <gnc_utilities.hpp>
#include <gnc_constants.hpp>
#include <cmath>

constexpr float nan_f = std::numeric_limits<float>::quiet_NaN();

ADCSCommander::ADCSCommander(StateFieldRegistry& registry, unsigned int offset) :
    TimedControlTask<void>(registry, "attitude_computer", offset),
    filter_sr(0,1,8),
    rwa_mode_f("adcs_cmd.rwa_mode", Serializer<unsigned char>(2)),
    rwa_speed_cmd_f("adcs_cmd.rwa_speed_cmd", Serializer<f_vector_t>(rwa::min_speed_command,rwa::max_speed_command, 16*3)),
    rwa_torque_cmd_f("adcs_cmd.rwa_torque_cmd", Serializer<f_vector_t>(rwa::min_torque, rwa::max_torque, 16*3)),
    rwa_speed_filter_f("adcs_cmd.rwa_speed_filter", filter_sr),
    rwa_ramp_filter_f("adcs_cmd.rwa_ramp_filter", filter_sr),
    mtr_mode_f("adcs_cmd.mtr_mode", Serializer<unsigned char>(2)),
    mtr_cmd_f("adcs_cmd.mtr_cmd", Serializer<f_vector_t>(mtr::min_moment, mtr::max_moment, 16*3)),
    mtr_limit_f("adcs_cmd.mtr_limit", Serializer<float>(mtr::min_moment, mtr::max_moment, 16)),
    ssa_voltage_filter_f("adcs_cmd.ssa_voltage_filter", filter_sr),
    imu_mode_f("adcs_cmd.imu_mode", Serializer<unsigned char>(4)),
    imu_mag_filter_f("adcs_cmd.imu_mag_filter", filter_sr),
    imu_gyr_filter_f("adcs_cmd.imu_gyr_filter", filter_sr),
    imu_gyr_temp_filter_f("adcs_cmd.imu_gyr_temp_filter", filter_sr),
    k_sr(std::numeric_limits<float>::min(), std::numeric_limits<float>::max(),16),
    imu_gyr_temp_kp_f("adcs_cmd.imu_temp_kp", k_sr),
    imu_gyr_temp_ki_f("adcs_cmd.imu_temp_ki", k_sr),
    imu_gyr_temp_kd_f("adcs_cmd.imu_temp_kd", k_sr),
    imu_gyr_temp_desired_f("adcs_cmd.imu_gyr_temp_desired", Serializer<float>(imu::min_eq_temp, imu::max_eq_temp, 8)),
    havt_bool_sr()
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
    havt_cmd_table_vector_f.reserve(adcs_havt::Index::_LENGTH);
    // fill vector of statefields for cmd havt
    char buffer[50];
    for (unsigned int idx = adcs_havt::Index::IMU_GYR; idx < adcs_havt::Index::_LENGTH; idx++ )
    {
    std::memset(buffer, 0, sizeof(buffer));
    sprintf(buffer,"adcs_cmd.havt_device");
    sprintf(buffer + strlen(buffer), "%u", idx);
    havt_cmd_table_vector_f.emplace_back(buffer, havt_bool_sr);
    add_writable_field(havt_cmd_table_vector_f[idx]);
    }

    // find adcs state, will be used evnetually?
    adcs_state_fp = find_writable_field<unsigned char>("adcs.state", __FILE__, __LINE__);
    
    // Initialize outputs to NaN values
    adcs_vec1_current_f.set({nan_f, nan_f, nan_f});
    adcs_vec2_current_f.set({nan_f, nan_f, nan_f});
    adcs_vec1_desired_f.set({nan_f, nan_f, nan_f});
    adcs_vec2_desired_f.set({nan_f, nan_f, nan_f});
}

void ADCSCommander::execute() {

}
