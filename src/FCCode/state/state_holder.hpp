/** @file state_holder.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains declarations for state data that can be used by the rest
 * of the flight code.
 */

#ifndef MASTER_STATE_HOLDER_H_
#define MASTER_STATE_HOLDER_H_

#include <map>
#include <rwmutex.hpp>
#include <Gomspace/Gomspace.hpp>
#include "state_definitions.hpp"
#include "device_states.hpp"
#include "../controllers/controllers.hpp"
#include "../comms/uplink_struct.hpp"

namespace State {
  namespace Master {
    //! Master controller state
    extern MasterState master_state;
    //! PAN-specific master controller state
    extern PANState pan_state; 
    //! Current boot # of satellite
    extern unsigned int boot_number; 
    //! Last time that data was uplinked to satellite
    extern msg_gps_time_t last_uplink_time; 
    //! If last uplink was parseable by satellite
    extern bool was_last_uplink_valid; 
    //! Did the hardware setup succeed?
    extern bool is_deployed; 
    //! True if this satellite is leader.
    extern bool is_follower;
    //! Is set to true when safehold auto-exists. We use this to prevent
    //! standby mode from sending the system right back into safe hold after
    //! the timer expires. When processing an uplink, this flag should be set 
    //! to false so that we can reenter safe hold when an error occurs.
    extern bool autoexited_safe_hold; // TODO set to false in uplink processor
    //! Readers-writers lock that prevents multi-process modification of the master state.
    extern rwmutex_t master_state_lock;
  }

  namespace ADCS {
    //! Finite State Machine state of ADCS system.
    extern ADCSState adcs_state;
    //! If mode is "pointing", the currently commanded attitude as a quaternion
    extern std::array<float, 4> cmd_attitude;
    //! If mode is "pointing", the currently commanded angular rate as a vector in body frame
    extern std::array<float, 3> cmd_ang_rate;
    //! Current attitude as a quaternion
    extern std::array<float, 4> cur_attitude;
    //! Current angular rate as a vector in body frame
    extern std::array<float, 3> cur_ang_rate; 
    //! Has propulsion taken over ADCS for the sake of efficient thrust manuevers?
    extern bool is_propulsion_pointing_active; 
    //! Is the sun vector determination working?
    extern bool is_sun_vector_determination_working;
    //! Are we able to collect the sun vector? i.e. the ADCS system is not timing out on determining
    // a sun vector, right?
    extern bool is_sun_vector_collection_working;
    //! Maximum angular rate magnitude that is considered "stable".
    constexpr float MAX_STABLE_ANGULAR_RATE = 0.0f; // TODO set value
    //! Maximum angular rate magnitude that is considered "semistable", e.g. not perfectly stable, but OK for emergency communication.
    constexpr float MAX_SEMISTABLE_ANGULAR_RATE = 0.0f; // TODO set value
    /**
     * @brief Computes the magnitude of the angular rate of the spacecraft based on the cur_ang_rate vector.
     * @return float The current angular rate of the spacecraft.
     */
    float angular_rate();
    
    /** Lower-level ADCS data **/
    //! Most recent reaction wheel speed commands
    extern std::array<float, 3> rwa_speed_cmds; 
    //! Most recent reaction wheel speeds
    extern std::array<float, 3> rwa_speeds; 
    //! Most recent reaction wheel ramp values
    extern std::array<float, 3> rwa_ramps; 
    //! Spacecraft angular momentum (computed from reaction wheel speeds)
    extern std::array<float, 3> spacecraft_L; 
    //! The most recent torque command for reaction wheels
    extern std::array<float, 3> rwa_torque_cmds; 
    //! The most recent magnetorquer command
    extern std::array<float, 3> mtr_cmds; 
    //! Vector pointing to sun sensor in body frame
    extern std::array<float, 3> ssa_vec; 
    //! IMU raw gyroscope data
    extern std::array<float, 3> gyro_data; 
    //! IMU raw magnetometer data
    extern std::array<float, 3> mag_data; 
    //! Raw sun sensor data. Infrequently accessed. 
    extern std::array<float, 20> ssa_adc_data;
    //! Readers-writers lock that prevents multi-process modification of ADCS state data.
    extern rwmutex_t adcs_state_lock;
  }

  namespace Gomspace {
    //! Gomspace housekeeping data.
    extern Devices::Gomspace::eps_hk_t gomspace_data;
    //! The value of the "config" field for Gomspace.
    extern Devices::Gomspace::eps_config_t gomspace_config;
    //! The value of the "config2" field for Gomspace.
    extern Devices::Gomspace::eps_config2_t gomspace_config2;
    //! Readers-writers lock that prevents multi-process modification of Gomspace state data.
    extern rwmutex_t gomspace_state_lock;
  }

  namespace Propulsion {
    //! Available delta-v, in meters/second. Note that this is obtained by simply integrating
    // the applied thrusts over time, so this number may not be accurate. Propulsion leaks, for example,
    // would contribute to significant inaccuracy. The purpose of this field is to serve as an upper
    // bound.
    extern float delta_v_available;
    //! Are propulsive manuevers enabled?
    extern bool is_propulsion_enabled;
    //! Is there a currently planned firing?
    extern bool is_firing_planned;
    //! Is the currently planned firing due to a previous uplink?
    extern bool is_firing_planned_by_uplink;
    //! Is the propulsion state controller currently repressurizing the tank?
    extern bool is_repressurization_active;
    //! Firing data for an upcoming planned firing.
    extern Firing firing_data;
    //! Current pressure within tank.
    extern float tank_pressure;
    //! Current temperature within inner tank.
    extern float tank_inner_temperature;
    //! Current temperature within outer tank.
    extern float tank_outer_temperature;
    //! Readers-writers lock that prevents multi-process modification of propulsion state data.
    extern rwmutex_t propulsion_state_lock; // TODO There may be LOCK CONTENTION on this lock due to both the propulsion and
                                          // master processes trying to write to it! Handle this lock contention carefully.
  }

  namespace Piksi {
    //! Current time in GPS format.
    extern msg_gps_time_t current_time;
    //! Most recent GPS position.
    extern std::array<double, 3> gps_position;
    //! Most recent GPS velocity.
    extern std::array<double, 3> gps_velocity;
    //! Readers-writers lock that prevents multi-process modification of Piksi state data.
    extern rwmutex_t piksi_state_lock;
  }

  namespace Quake {
    //! Struct containing data of most recent uplink
    extern Comms::Uplink most_recent_uplink;
    //! Readers-writers lock that prevents multi-process modification of uplink data.
    extern rwmutex_t uplink_lock;
    //! Tracks number of consecutive missed uplinks from ground.
    extern unsigned int missed_uplinks;
    #ifdef DEBUG
    //! If the number of missed uplinks exceeds this value, safe hold is triggered.
    static constexpr unsigned int MAX_MISSED_UPLINKS = 5; // Approximately 2.5 minutes
    #else
    //! If the number of missed uplinks exceeds this value, safe hold is triggered.
    static constexpr unsigned int MAX_MISSED_UPLINKS = 288; // Approximately 24 hours
    #endif
    //! Readers-writers lock that prevents multi-process modification of Quake state data.
    extern rwmutex_t quake_state_lock;
  }
}

#endif