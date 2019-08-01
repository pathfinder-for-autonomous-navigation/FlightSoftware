#include "fault_state_holder.hpp"

namespace FaultState {
namespace Gomspace {
rwmutex_t gomspace_faults_state_lock;
std::bitset<GOMSPACE_FAULTS::NUMBER_FAULTS> fault_bits;
bool vbatt_ignored = false;
}  // namespace Gomspace
namespace ADCS {
bool all_magnetometers_faulty_ignore = false;
bool all_ssa_faulty_ignore = false;
bool motor_x_faulty_ignore = false;
bool motor_y_faulty_ignore = false;
bool motor_z_faulty_ignore = false;
rwmutex_t adcs_faults_state_lock;
}  // namespace ADCS
namespace Propulsion {
gps_time_t overpressure_event;
gps_time_t destabilization_event;
OVERPRESSURE_EVENT overpressure_event_id;
bool cannot_pressurize_outer_tank = false;
bool cannot_pressurize_outer_tank_ignored = false;
rwmutex_t propulsion_faults_state_lock;
}  // namespace Propulsion
}  // namespace FaultState