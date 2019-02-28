/** @file state_holder.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains declarations for state data that can be used by the rest
 * of the flight code.
 */

#ifndef FAULT_STATE_HOLDER_H_
#define FAULT_STATE_HOLDER_H_

#include <rwmutex.hpp>

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