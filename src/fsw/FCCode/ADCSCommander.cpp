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
    add_writable_field(adcs_vec1_current_f);
    add_writable_field(adcs_vec1_desired_f);
    add_writable_field(adcs_vec2_current_f);
    add_writable_field(adcs_vec2_desired_f);

    adcs_state_fp = find_writable_field<unsigned char>("adcs.state", __FILE__, __LINE__);
    q_body_eci_fp = find_readable_field<f_quat_t>("attitude_estimator.q_body_eci", __FILE__, __LINE__);
    ssa_vec_fp = find_readable_field<f_vector_t>("adcs_monitor.ssa_vec", __FILE__, __LINE__);
    pos_fp = find_readable_field<d_vector_t>("orbit.pos", __FILE__, __LINE__);
    baseline_pos_fp = find_readable_field<d_vector_t>("orbit.baseline_pos", __FILE__, __LINE__);

    // Initialize outputs to NaN values
    adcs_vec1_current_f.set({nan_f, nan_f, nan_f});
    adcs_vec2_current_f.set({nan_f, nan_f, nan_f});
    adcs_vec1_desired_f.set({nan_f, nan_f, nan_f});
    adcs_vec2_desired_f.set({nan_f, nan_f, nan_f});
}

void ADCSCommander::execute() {

}
