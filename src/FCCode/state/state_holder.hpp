/** @file state_holder.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains declarations for state data that can be used by the rest
 * of the flight code.
 */

#ifndef STATE_HOLDER_HPP_
#define STATE_HOLDER_HPP_

#include <map>
#include <rwmutex.hpp>
#include <Gomspace/Gomspace.hpp>
#include <AttitudeMath.hpp>
#include <Piksi/GPSTime.hpp>
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
    extern gps_time_t last_uplink_time; 
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
    //! Current attitude as a quaternion
    extern std::array<float, 4> cur_attitude;
    //! Current angular rate as a vector in body frame
    extern std::array<float, 3> cur_ang_rate; 
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
    inline float angular_rate() {
      return vect_mag(cur_ang_rate.data());
    }
    
    /** Lower-level ADCS data **/
    //! Most recent reaction wheel speed commands
    extern std::array<float, 3> rwa_speed_cmds; 
    //! Most recent reaction wheel speeds
    extern std::array<float, 3> rwa_speeds; 
    //! Most recent reaction wheel ramp values
    extern std::array<float, 3> rwa_ramps; 
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
    //! State of propulsion state controller.
    extern PropulsionState propulsion_state;
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

  namespace GNC {
      //! Most recent GPS position, as last obtained from the orbit propagator.
      extern std::array<double, 3> gps_position;
      //! Most recently expected GPS position of other satellite, as last obtained from the orbit propagator.
      extern std::array<double, 3> gps_position_other;
      //! Most recent GPS velocity, as last obtained from the orbit propagator.
      extern std::array<double, 3> gps_velocity;
      //! Most recent GPS velocity of other satellite, as last obtained from the orbit propagator.
      extern std::array<double, 3> gps_velocity_other;
      //! Tracks whether or not a firing has happened during the current nighttime period.
      extern bool has_firing_happened_in_nighttime;
      //! Readers-writers lock that prevents multi-process modification of GNC state data.
      extern rwmutex_t gnc_state_lock;
  }

  namespace Piksi {
    // TODO write GPS time and position to EEPROM every few seconds, so that in the event 
    // of a reboot the satellite can still roughly know where it is.

    //! Current time in GPS format, as last obtained from Piksi.
    extern gps_time_t recorded_current_time;
    //! Timestamp at which current time was collected from Piksi.
    extern systime_t time_collection_timestamp;
    //! Most recent GPS position, as last obtained from Piksi.
    extern std::array<double, 3> recorded_gps_position;
    //! Most recently expected GPS position of other satellite, as last obtained from Piksi or ground.
    extern std::array<double, 3> recorded_gps_position_other;
    //! Most recent GPS velocity, as last obtained from Piksi.
    extern std::array<double, 3> recorded_gps_velocity;
    //! Most recent GPS velocity of other satellite, as last obtained from Piksi or ground.
    extern std::array<double, 3> recorded_gps_velocity_other;
    //! Readers-writers lock that prevents multi-process modification of Piksi state data.
    extern rwmutex_t piksi_state_lock;

    //! Current propagated GPS time. Propagation occurs on each call of current_time(). 
    // This field needs to be updated every time GPS time is actually collected.
    extern gps_time_t propagated_current_time;
    //! Current propagated GPS time collection timestamp. Propagation occurs on each call of current_time().
    // This field needs to be updated every time GPS time is actually collected.
    extern systime_t propagated_time_collection_timestamp;
    //! Function to report (propagated) current time.
    inline gps_time_t current_time() {
      systime_t current_systime = chVTGetSystemTimeX();
      systime_t systime_delta = current_systime - propagated_time_collection_timestamp;
      propagated_time_collection_timestamp = current_systime;
      rwMtxRLock(&piksi_state_lock);
        propagated_current_time = propagated_current_time + MS2ST(systime_delta);
      rwMtxRUnlock(&piksi_state_lock);
      return propagated_current_time;
    }
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