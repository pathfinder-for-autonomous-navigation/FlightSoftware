/** @file state_holder.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains initializations for state data.
 */

#include "state_holder.hpp"

namespace State {
    // Default values for all state-related data
    namespace Master {
        Master::MasterState master_state = Master::MasterState::STARTUP;
        Master::PANState pan_state = Master::PANState::MASTER_STARTUP;
        unsigned int boot_number = 1;
        msg_gps_time_t last_uplink_time;
        bool was_last_uplink_valid = false;
        bool is_deployed = false;
        bool is_follower = false;
        bool autoexited_safe_hold = false;
        rwmutex_t master_state_lock;
    }

    namespace ADCS {
        ADCS::ADCSState adcs_state = ADCS::ADCSState::ADCS_SAFE_HOLD;

        static Hardware::DeviceState adcs_motorpot_state = {false, false, false};
        static Hardware::DeviceState adcs_motor_x_state = {false, false, false};
        static Hardware::DeviceState adcs_motor_y_state = {false, false, false};
        static Hardware::DeviceState adcs_motor_z_state = {false, false, false};
        static Hardware::DeviceState adcs_adc_motor_x_state = {false, false, false};
        static Hardware::DeviceState adcs_adc_motor_y_state = {false, false, false};
        static Hardware::DeviceState adcs_adc_motor_z_state = {false, false, false};
        static Hardware::DeviceState adcs_magnetorquer_x_state = {false, false, false};
        static Hardware::DeviceState adcs_magnetorquer_y_state = {false, false, false};
        static Hardware::DeviceState adcs_magnetorquer_z_state = {false, false, false};
        static Hardware::DeviceState adcs_ssa_adc_1_state = {false, false, false};
        static Hardware::DeviceState adcs_ssa_adc_2_state = {false, false, false};
        static Hardware::DeviceState adcs_ssa_adc_3_state = {false, false, false};
        static Hardware::DeviceState adcs_ssa_adc_4_state = {false, false, false};
        static Hardware::DeviceState adcs_ssa_adc_5_state = {false, false, false};
        static Hardware::DeviceState adcs_gyro_state = {false, false, false};
        static Hardware::DeviceState adcs_magnetometer_state = {false, false, false};
        std::map<std::string, Hardware::DeviceState&> adcs_hat {
            {"Motor Potentiometer", adcs_motorpot_state},
            {"Motor X", adcs_motor_x_state},
            {"Motor Y", adcs_motor_y_state},
            {"Motor Z", adcs_motor_z_state},
            {"Motor X ADC", adcs_adc_motor_x_state},
            {"Motor Y ADC", adcs_adc_motor_y_state},
            {"Motor Z ADC", adcs_adc_motor_z_state},
            {"Magnetorquer X", adcs_magnetorquer_x_state},
            {"Magnetorquer Y", adcs_magnetorquer_y_state},
            {"Magnetorquer Z", adcs_magnetorquer_z_state},
            {"Sun Sensor ADC 1", adcs_ssa_adc_1_state},
            {"Sun Sensor ADC 2", adcs_ssa_adc_2_state},
            {"Sun Sensor ADC 3", adcs_ssa_adc_3_state},
            {"Sun Sensor ADC 4", adcs_ssa_adc_4_state},
            {"Sun Sensor ADC 5", adcs_ssa_adc_5_state},
            {"Gyroscope", adcs_gyro_state},
            {"Magnetometer", adcs_magnetometer_state},
        };

        std::array<float, 4> cmd_attitude;
        std::array<float, 3> cmd_ang_rate;
        std::array<float, 4> cur_attitude;
        std::array<float, 3> cur_ang_rate;
        bool is_propulsion_pointing_active = false;
        bool is_sun_vector_determination_working = false;
        bool is_sun_vector_collection_working = false;
        float angular_rate() {
            return sqrt(pow(cur_ang_rate[0],2) + pow(cur_ang_rate[1],2) + pow(cur_ang_rate[2],2));
        }
        std::array<float, 3> rwa_speed_cmds, rwa_speeds, rwa_ramps;
        std::array<float, 3> spacecraft_L;
        std::array<float, 3> rwa_torque_cmds;
        std::array<float, 3> mtr_cmds;
        std::array<float, 3> ssa_vec;
        std::array<float, 3> gyro_data, mag_data;
        std::array<float, 20> ssa_adc_data;
        rwmutex_t adcs_state_lock;
    }

    namespace Gomspace {
        Devices::Gomspace::eps_hk_t gomspace_data;
        Devices::Gomspace::eps_config_t gomspace_config;
        Devices::Gomspace::eps_config2_t gomspace_config2;
        rwmutex_t gomspace_state_lock;
    }

    namespace Propulsion {
        bool is_firing_planned = false;
        bool is_firing_planned_by_uplink = false;
        bool is_propulsion_enabled = false;
        bool is_repressurization_active = false;
        bool is_propulsion_active = false;
        float delta_v_available = 11.0f; // m/s
        Firing firing_data = {{0.0,0.0,0.0},{0,0,0,0}};
        float tank_pressure = 0.0f;
        float tank_inner_temperature = 0.0f;
        float tank_outer_temperature = 0.0f;
        rwmutex_t propulsion_state_lock;
    }

    namespace Piksi {
        msg_gps_time_t current_time;
        std::array<float, 3> gps_position, gps_velocity;
        rwmutex_t piksi_state_lock;
    }

    namespace Quake {
        Comms::Uplink most_recent_uplink;
        unsigned int missed_uplinks = 0;
        rwmutex_t quake_state_lock;
        rwmutex_t uplink_lock;
    }
}
