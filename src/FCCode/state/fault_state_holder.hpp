/** @file state_holder.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains declarations for state data that can be used by the rest
 * of the flight code.
 */

#ifndef FAULT_STATE_HOLDER_H_
#define FAULT_STATE_HOLDER_H_

#include <rwmutex.hpp>
#include <bitset>

namespace FaultState {
  namespace Master {
    //! Readers-writers lock that prevents multi-process modification of master fault state data.
    extern rwmutex_t master_fault_state_lock;
  }

  namespace ADCS {
    //! Readers-writers lock that prevents multi-process modification of ADCS fault state data.
    extern rwmutex_t adcs_fault_state_lock;
  }

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
    //! Whether or not the current voltage of the battery is less than the "safe hold" threshold
    extern bool is_safe_hold_voltage;
    //! Readers-writers lock that prevents multi-process modification of Gomspace fault state data.
    extern rwmutex_t gomspace_fault_state_lock;
  }

  namespace Propulsion {
    //! Readers-writers lock that prevents multi-process modification of propulsion fault state data.
    extern rwmutex_t propulsion_fault_state_lock;
  }

  namespace GNC {
      //! Readers-writers lock that prevents multi-process modification of GNC fault state data.
      extern rwmutex_t gnc_fault_state_lock;
  }

  namespace Piksi {
    //! Readers-writers lock that prevents multi-process modification of Piksi fault state data.
    extern rwmutex_t piksi_fault_state_lock;
  }

  namespace Quake {
    //! Readers-writers lock that prevents multi-process modification of Quake fault state data.
    extern rwmutex_t quake_fault_state_lock;
  }
}


#endif