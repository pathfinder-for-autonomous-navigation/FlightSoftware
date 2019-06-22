/** @file state_holder.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains initializations for state data.
 */

#include "state_holder.hpp"
#include "tensor.hpp"
#include "StateField.hpp"
#include "StateFieldRegistry.hpp"
#include "StateMachine.hpp"

namespace State {
    // Default values for all state-related data
    namespace Master {
        MasterState master_state = MasterState::STARTUP;
        PANState pan_state = PANState::MASTER_STARTUP;
        unsigned int boot_number = 1;
        gps_time_t last_uplink_time;
        bool uplink_command_applied = false;
        bool was_last_uplink_valid = false;
        bool is_deployed = false;
        bool is_follower = false;
        bool docking_switch_pressed = false;
        bool autoexited_safe_hold = false;
        rwmutex_t master_state_lock;
    }

    namespace ADCS {
        // TODO add exciting new state fields!
        ADCSState adcs_state = ADCSState::ADCS_SAFE_HOLD;
        std::array<float, 4> cmd_attitude;
        PointingFrame cmd_attitude_frame = PointingFrame::ECI;
        std::array<float, 4> cur_attitude;
        std::array<float, 3> cur_ang_rate;
        bool is_sun_vector_determination_working = false;
        bool is_sun_vector_collection_working = false;
        std::array<float, 3> rwa_speed_cmds, rwa_speeds, rwa_torques;
        std::array<float, 3> rwa_speed_cmds_rd, rwa_speeds_rd, rwa_ramps_rd;
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
        PropulsionState propulsion_state = PropulsionState::DISABLED;
        Firing firing_data;
        float tank_pressure = 0.0f;
        float tank_inner_temperature = 0.0f;
        float tank_outer_temperature = 0.0f;
        unsigned char intertank_firing_valve = Devices::SpikeAndHold::INTERTANK_MAIN;
        rwmutex_t propulsion_state_lock;
    }

    namespace GNC {
        std::array<double, 3> gps_position, gps_position_other, gps_velocity, gps_velocity_other;
        std::array<float, 4> ecef_to_eci;
        std::array<float, 4> eci_to_lvlh;
        gps_time_t current_time;
        systime_t time_collection_timestamp;
        bool has_firing_happened_in_nighttime = false;
        rwmutex_t gnc_state_lock;

        double distance() {
            std::array<double, 3> pos = State::read(gps_position, gnc_state_lock);
            std::array<double, 3> pos_other = State::read(gps_position_other, gnc_state_lock);
            pla::Vec3d dpos;
            for(int i = 0; i < 3; i++) dpos[i] = pos[i] - pos_other[i];
            return dpos.length();
        }

        gps_time_t get_current_time() {
            systime_t current_systime = chVTGetSystemTimeX();
            systime_t systime_delta = current_systime - time_collection_timestamp;
            time_collection_timestamp = current_systime;
            rwMtxRLock(&State::Piksi::piksi_state_lock);
                current_time = current_time + MS2ST(systime_delta);
            rwMtxRUnlock(&State::Piksi::piksi_state_lock);
            return current_time;
        }
    }

    namespace Piksi {
        gps_time_t recorded_current_time;
        systime_t recorded_time_collection_timestamp;
        std::array<double, 3> recorded_gps_position, recorded_gps_position_other, recorded_gps_velocity;
        gps_time_t recorded_gps_position_time, recorded_gps_position_other_time, recorded_gps_velocity_time;
        unsigned char recorded_gps_position_nsats, recorded_gps_velocity_nsats;
        bool is_float_rtk, is_fixed_rtk;
        rwmutex_t piksi_state_lock;
    }

    namespace Quake {
        Comms::Uplink most_recent_uplink;
        gps_time_t sbdix_time_received;
        rwmutex_t uplink_lock;
        QuakeState quake_state = QuakeState::WAITING;
        rwmutex_t quake_state_lock;

        circular_stack<QuakeMessage, MAX_DOWNLINK_HISTORY> downlink_stack;
    }
}
