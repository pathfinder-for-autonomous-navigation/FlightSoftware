/** @file state_holder.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains declarations for state data that can be used by the rest
 * of the flight code.
 */

#ifndef FAULT_STATE_HOLDER_H_
#define FAULT_STATE_HOLDER_H_

#include <rwmutex.hpp>
#include <GPSTime.hpp>
#include <bitset>

namespace FaultState {
  namespace Gomspace {
    enum GOMSPACE_FAULTS {
      BATTERY_VOLTAGE,
      BOOST_VOLTAGE_1,
      BOOST_VOLTAGE_2,
      BOOST_VOLTAGE_3,
      BOOST_CURRENT_1,
      BOOST_CURRENT_2,
      BOOST_CURRENT_3,
      BOOST_CURRENT_TOTAL,
      OUTPUT_PIKSI_TOGGLED,
      OUTPUT_PIKSI_CURRENT,
      OUTPUT_QUAKE_TOGGLED,
      OUTPUT_QUAKE_CURRENT,
      OUTPUT_ADCS_TOGGLED,
      OUTPUT_ADCS_CURRENT,
      OUTPUT_SPIKE_AND_HOLD_TOGGLED,
      OUTPUT_SPIKE_AND_HOLD_CURRENT,
      BATTERY_CURRENT,
      TEMPERATURE_1,
      TEMPERATURE_2,
      TEMPERATURE_3,
      TEMPERATURE_4,
      NUMBER_FAULTS = TEMPERATURE_4
    };
    //! Stores current faults of Gomspace system
    extern std::bitset<GOMSPACE_FAULTS::NUMBER_FAULTS> fault_bits;
    //! Whether or not a low-battery error is ignored
    extern bool vbatt_ignored;
    //! Readers-writers lock that prevents multi-process modification of Gomspace fault state data.
    extern rwmutex_t gomspace_faults_state_lock;
  }

  namespace Propulsion {
    //! GPS time of when an overpressure event occurred.
    extern gps_time_t overpressure_event;
    //! Type of condition that triggered overpressure event.
    enum OVERPRESSURE_EVENT {
      NONE,
      INNER_TANK_TEMPERATURE,
      OUTER_TANK_TEMPERATURE,
      OUTER_TANK_PRESSURE
    };
    extern OVERPRESSURE_EVENT overpressure_event_id;
    //! GPS time of a firing that was cancelled due to unstable pointing.
    extern gps_time_t destabilization_event;
    //! If true, we are unable to pressurize the outer tank up to the requisite pressure for a firing.
    extern bool cannot_pressurize_outer_tank;
    //! Ignore leaking error.
    extern bool cannot_pressurize_outer_tank_ignored;
    //! Readers-writers lock that prevents multi-process modification of propulsion fault state data.
    extern rwmutex_t propulsion_faults_state_lock;
  }
}


#endif