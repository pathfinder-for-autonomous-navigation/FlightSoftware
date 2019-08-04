/** THIS IS AN AUTOGENERATED FILE **/
#ifndef UPLINK_STRUCT_HPP_
#define UPLINK_STRUCT_HPP_

#include <GPSTime.hpp>
#include <array>

namespace Comms {
constexpr unsigned int UPLINK_SIZE_BITS = 522;
struct Uplink {
    unsigned int uplink_number;
    std::array<double, 3> other_satellite_position;
    std::array<double, 3> other_satellite_velocity;
    gps_time_t other_satellite_timestamp;
    unsigned int constant_0_id;
    unsigned int constant_0_val;
    unsigned int constant_1_id;
    unsigned int constant_1_val;
    unsigned int constant_2_id;
    unsigned int constant_2_val;
    unsigned int constant_3_id;
    unsigned int constant_3_val;
    unsigned int constant_4_id;
    unsigned int constant_4_val;
    unsigned int master_state;
    unsigned int pan_state;
    bool is_follower;
    bool fc_hat_gomspace;
    bool fc_hat_piksi;
    bool fc_hat_quake;
    bool fc_hat_dcdc;
    bool fc_hat_spike_and_hold;
    bool fc_hat_adcs_system;
    bool fc_hat_pressure_sensor;
    bool fc_hat_temp_sensor_inner;
    bool fc_hat_temp_sensor_outer;
    bool fc_hat_docking_motor;
    bool fc_hat_docking_switch;
    bool adcs_hat_gyroscope;
    bool adcs_hat_magnetometer_1;
    bool adcs_hat_magnetometer_2;
    bool adcs_hat_magnetorquer_x;
    bool adcs_hat_magnetorquer_y;
    bool adcs_hat_magnetorquer_z;
    bool adcs_hat_motorpot;
    bool adcs_hat_motor_x;
    bool adcs_hat_motor_y;
    bool adcs_hat_motor_z;
    bool adcs_hat_adc_motor_x;
    bool adcs_hat_adc_motor_y;
    bool adcs_hat_adc_motor_z;
    bool adcs_hat_ssa_adc_1;
    bool adcs_hat_ssa_adc_2;
    bool adcs_hat_ssa_adc_3;
    bool adcs_hat_ssa_adc_4;
    bool adcs_hat_ssa_adc_5;
    bool vbatt_ignored;
    bool cannot_pressurize_outer_tank_ignored;
    bool all_magnetometers_faulty_ignore;
    bool all_ssa_faulty_ignore;
    bool motor_x_faulty_ignore;
    bool motor_y_faulty_ignore;
    bool motor_z_faulty_ignore;
    bool ignore_destabilized;
    bool ignore_overpressure;
    unsigned int adcs_state;
    unsigned int adcs_gain_state;
    bool command_adcs;
    unsigned int adcs_frame;
    std::array<float, 4> adcs_attitude;
    bool command_propulsion;
    std::array<float, 3> firing_vector;
    gps_time_t firing_time;
    bool docking_motor_mode;
    bool reset_piksi;
    bool reset_quake;
    bool reset_dcdc;
    bool reset_spike_and_hold;
    bool power_cycle_gomspace;
    bool power_cycle_piksi;
    bool power_cycle_quake;
    bool power_cycle_adcs_system;
    bool power_cycle_spike_and_hold;
};
}  // namespace Comms
#endif