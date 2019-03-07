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
#include <GPSTime.hpp>
#include <circular_stack.hpp>
#include "state_definitions.hpp"
#include "device_states.hpp"
#include "../controllers/controllers.hpp"
#include "../comms/uplink_struct.hpp"

namespace State {
  //! Helper function to read from state variables in a protected way
  template<typename T>
  inline T read(const T& val, rwmutex_t& lock) {
    T val_cpy;
    rwMtxRLock(&lock);
      val_cpy = val;
    rwMtxRUnlock(&lock);
    return val_cpy;
  }

  //! Helper function to write to state variables in a protected way
  template<typename T>
  inline void write(T& val, const T& new_val, rwmutex_t& lock) {
    rwMtxWLock(&lock);
      val = new_val;
    rwMtxWUnlock(&lock);
  }

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
    //! Most recent reaction wheel speed commands, as read from ADCS
    extern std::array<float, 3> rwa_speed_cmds_rd; 
    //! Most recent reaction wheel speeds, as read from ADCS
    extern std::array<float, 3> rwa_speeds_rd; 
    //! Most recent reaction wheel ramp values, as read from ADCS
    extern std::array<float, 3> rwa_ramps_rd;
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
    //! Preferred valve for venting between tanks. Can be changed by ground.
    extern unsigned char intertank_firing_valve;
    //! Readers-writers lock that prevents multi-process modification of propulsion state data.
    extern rwmutex_t propulsion_state_lock;
  }

  namespace Piksi {
    //! Current time in GPS format, as last obtained from Piksi.
    extern gps_time_t recorded_current_time;
    //! Timestamp at which current time was collected from Piksi.
    extern systime_t recorded_time_collection_timestamp;
    //! Most recent GPS position, as last obtained from Piksi
    extern std::array<double, 3> recorded_gps_position;
    //! GPS timestamp at which most recent position was collected.
    extern gps_time_t recorded_gps_position_time;
    //! Number of satellites used in position determination.
    extern unsigned char recorded_gps_position_nsats;
    //! Most recently expected GPS position of other satellite, as last obtained from Piksi or ground.
    extern std::array<double, 3> recorded_gps_position_other;
    //! GPS timestamp at which most recent position of other satellite was collected.
    extern gps_time_t recorded_gps_position_other_time;
    //! Most recent GPS velocity, as last obtained from Piksi.
    extern std::array<double, 3> recorded_gps_velocity;
    //! GPS timestamp at which most recent velocity was collected.
    extern gps_time_t recorded_gps_velocity_time;
    //! Number of satellites used in velocity determination.
    extern unsigned char recorded_gps_velocity_nsats;
    //! Readers-writers lock that prevents multi-process modification of Piksi state data.
    extern rwmutex_t piksi_state_lock;
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
    //! Quaternion representing rotation from ECEF to ECI.
    extern std::array<double, 4> ecef_to_eci;
    //! Current propagated GPS time. Propagation occurs on each call of current_time(). 
    // This field needs to be updated every time GPS time is actually collected.
    extern gps_time_t current_time;
    //! Current propagated GPS time collection timestamp. Propagation occurs on each call of current_time().
    // This field needs to be updated every time GPS time is actually collected.
    extern systime_t time_collection_timestamp;
    //! Function to report (propagated) current time.
    inline gps_time_t get_current_time() {
      systime_t current_systime = chVTGetSystemTimeX();
      systime_t systime_delta = current_systime - time_collection_timestamp;
      time_collection_timestamp = current_systime;
      rwMtxRLock(&State::Piksi::piksi_state_lock);
        current_time = current_time + MS2ST(systime_delta);
      rwMtxRUnlock(&State::Piksi::piksi_state_lock);
      return current_time;
    }
    //! Tracks whether or not a firing has happened during the current nighttime period.
    extern bool has_firing_happened_in_nighttime;
    //! Readers-writers lock that prevents multi-process modification of GNC state data.
    extern rwmutex_t gnc_state_lock;
  }

  namespace Quake {
    //! Struct containing most recent uplink data received by satellite
    extern Comms::Uplink most_recent_uplink;
    //! Readers-writers lock that prevents multi-process modification of most recent uplink
    extern rwmutex_t uplink_lock;
    //! State of finite state machine that controls the Quake controller
    extern QuakeState quake_state;
    //! Readers-writers lock that prevents multi-process modification of Quake state data.
    extern rwmutex_t quake_state_lock;

    constexpr unsigned int PACKET_LENGTH = 70;
    constexpr unsigned int PACKETS_PER_DOWNLINK = 10;
    typedef std::array<Devices::QLocate::Message, PACKETS_PER_DOWNLINK> full_data_downlink;
    //! Maximum number of data packets to store in history.
    // TODO store most recent packets in EEPROM as well.
    constexpr unsigned int MAX_DOWNLINK_HISTORY = 25;
    //! Packets are automatically added to this stack by the consumer threads if they were 
    // not forcibly required to produce a partial packet.
    extern circular_stack<full_data_downlink, MAX_DOWNLINK_HISTORY> downlink_stack; 
    //! If the producer threads were forcibly interrupted, this is where they will dump their
    // latest packet.
    extern full_data_downlink most_recent_downlink;
  }
}

#endif