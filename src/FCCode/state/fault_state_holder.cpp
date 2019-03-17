#include "fault_state_holder.hpp"

namespace FaultState {
    namespace Gomspace {
        rwmutex_t gomspace_faults_state_lock;
        std::bitset<GOMSPACE_FAULTS::NUMBER_FAULTS> fault_bits;
        bool vbatt_ignored = false;
    }
    namespace Propulsion {
        gps_time_t overpressure_event;
        gps_time_t destabilization_event;
        OVERPRESSURE_EVENT overpressure_event_id;
        rwmutex_t propulsion_faults_state_lock;
        bool cannot_pressurize_outer_tank = false;
        bool cannot_pressurize_outer_tank_ignored = false;
    }
}