#include "fault_state_holder.hpp"

namespace FaultState {
    namespace Gomspace {
        rwmutex_t gomspace_fault_state_lock;
        std::bitset<GOMSPACE_FAULTS::NUMBER_FAULTS> fault_bits;
        bool is_safe_hold_voltage = false;
    }
}