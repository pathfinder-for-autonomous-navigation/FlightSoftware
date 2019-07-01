/** @file state_definitions.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains definitions for enums used within the flight controller state.
 */

#ifndef STATE_DEFINITIONS_HPP_
#define STATE_DEFINITIONS_HPP_

#include <Piksi.hpp>
#include <GPSTime.hpp>

// Containers and enums for holding state
namespace State {
  // Master
  namespace Master {
    //! State of finite-state machine that's part of the master controller
    enum MasterState {
      // Common Cubesat Modes
      STARTUP, // Startup/deployment mode
      INITIALIZATION_HOLD, // Startup/deployment mode
      DETUMBLE,
      SAFE_HOLD,
      NORMAL
    };
    //! PAN-Specific state of finite-state machine that's part of the master controller
    enum PANState {
      // Common states.
      // Used when satellite is not
      // ready to enter the PAN phase of the mission yet.
      MASTER_STARTUP,
      MASTER_INITIALIZATIONHOLD,
      MASTER_DETUMBLE,
      MASTER_SAFEHOLD,
      // Actually used by both satellites
      DOCKING,
      DOCKED,
      // Leader states
      STANDBY,
      LEADER_CLOSE_APPROACH,
      SPACEJUNK,
      // Follower states
      FOLLOWER,
      FOLLOWER_CLOSE_APPROACH,
      PAIRED
    };
  }

  namespace Hardware {
    //! hat and global device-related state.
    struct DeviceState {
      //! Device is being supplied power
      bool powered_on;
      //! Device is enabled
      bool enabled;
      //! Field is set by safe hold check
      bool is_functional;
      //! If true, the software ignores that this device is nonfunctional and tries to use it anyway
      bool error_ignored;
      //! Number of times this device has been booted up
      unsigned int boot_count;
    };
  }

  namespace ADCS {
    //! State of finite-state machine that's part of the ADCS controller
    enum ADCSState {
      ZERO_TORQUE,
      ADCS_DETUMBLE,
      POINTING,
      ADCS_SAFE_HOLD
    };
    //! Gains can either be set for an individual satellite or a dual-configuration satellite
    enum ADCSGainState {
      NORMAL,
      PAIRED
    };
    //! Available frames for specifying a command atittude
    enum PointingFrame {
      ECI,
      LVLH
    };
  }

  namespace Propulsion {
    enum PropulsionState {
      DISABLED,
      IDLE,
      VENTING,
      AWAITING_PRESSURIZATION,
      PRESSURIZING,
      FIRING
    };
    //! Container for data that represents a propulsion manuever.
    struct Firing {
      //! Vector in inertial frame specifying thrust impulse
      std::array<float, 3> impulse_vector;
      //! GPS time of thrust
      gps_time_t time; 
    };
  }

  namespace Quake {
    enum QuakeState {
      WAITING,
      TRANSCEIVING
    };
  }
}

#endif