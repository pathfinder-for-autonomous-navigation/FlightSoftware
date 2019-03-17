#include "master_helpers.hpp"
#include "../../state/fault_state_holder.hpp"

using State::Master::master_state_lock;

// TODO add "ignore faults"
bool Master::safe_hold_needed() {
    bool autoexited_safe_hold = State::read(State::Master::autoexited_safe_hold, master_state_lock);
    if (autoexited_safe_hold) return false; // Don't set up safehold in this case.

    // Check battery voltage
    unsigned short int vbatt = State::read(State::Gomspace::gomspace_data.vbatt, State::Gomspace::gomspace_state_lock);
    bool vbatt_ignored = State::read(FaultState::Gomspace::vbatt_ignored, FaultState::Gomspace::gomspace_faults_state_lock);
    if (vbatt < Constants::Gomspace::SAFE_VOLTAGE || !vbatt_ignored)
        return true;

    // Check whether propulsion tank is leaking
    rwMtxRLock(&FaultState::Propulsion::propulsion_faults_state_lock);
        bool cannot_pressurize_tank = FaultState::Propulsion::cannot_pressurize_outer_tank;
        bool cannot_pressurize_tank_ignored = FaultState::Propulsion::cannot_pressurize_outer_tank_ignored;
    rwMtxRUnlock(&FaultState::Propulsion::propulsion_faults_state_lock);
    if (cannot_pressurize_tank || !cannot_pressurize_tank_ignored)
        return true;
    
    // Check ADCS HAT
    // Check if any of the 
    // Check if all sun sensors are faulty
    bool all_ssa_faulty = true;
    rwMtxRLock(&State::ADCS::adcs_state_lock);
        all_ssa_faulty &= !State::ADCS::adcs_hat.at("ssa_adc_1").is_functional;
        all_ssa_faulty &= !State::ADCS::adcs_hat.at("ssa_adc_1").is_functional;
        all_ssa_faulty &= !State::ADCS::adcs_hat.at("ssa_adc_1").is_functional;
        all_ssa_faulty &= !State::ADCS::adcs_hat.at("ssa_adc_1").is_functional;
        all_ssa_faulty &= !State::ADCS::adcs_hat.at("ssa_adc_1").is_functional;
    rwMtxRUnlock(&State::ADCS::adcs_state_lock);
    if (!all_ssa_faulty) return true;
    // Check if both magnetometers faulty
    
    return false;
}