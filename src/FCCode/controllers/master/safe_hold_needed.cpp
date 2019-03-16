#include "master_helpers.hpp"
#include "../../state/fault_state_holder.hpp"

using State::Master::master_state_lock;

bool Master::safe_hold_needed() {
    bool autoexited_safe_hold = State::read(State::Master::autoexited_safe_hold, master_state_lock);
    if (autoexited_safe_hold) return false; // Don't set up safehold in this case.

    // Check battery voltage
    unsigned short int vbatt = State::read(State::Gomspace::gomspace_data.vbatt, State::Gomspace::gomspace_state_lock);
    if (vbatt < Constants::Gomspace::SAFE_VOLTAGE)
        return true;

    // Check whether propulsion tank is leaking
    bool cannot_pressurize_tank = State::read(FaultState::Propulsion::cannot_pressurize_outer_tank, 
                                              FaultState::Propulsion::propulsion_faults_state_lock);
    if (cannot_pressurize_tank)
        return true;
    
    // TODO check ADCS HAT
    
    return false;
}