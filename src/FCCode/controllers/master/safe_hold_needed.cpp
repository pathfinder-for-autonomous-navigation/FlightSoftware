#include "master_helpers.hpp"
#include "../../state/fault_state_holder.hpp"

using State::Master::master_state_lock;

bool Master::safe_hold_needed() {
    bool autoexited_safe_hold = State::read(State::Master::autoexited_safe_hold, master_state_lock);
    if (autoexited_safe_hold) return false; // Don't set up safehold in this case.

    // Check battery voltage
    unsigned short int vbatt = State::read(State::Gomspace::gomspace_data.vbatt, State::Gomspace::gomspace_state_lock);
    bool vbatt_ignored = State::read(FaultState::Gomspace::vbatt_ignored, FaultState::Gomspace::gomspace_faults_state_lock);
    if (vbatt < Constants::Gomspace::SAFE_VOLTAGE || !vbatt_ignored)
        return true;
    // TODO add more battery conditions??

    // Check whether propulsion tank is leaking
    rwMtxRLock(&FaultState::Propulsion::propulsion_faults_state_lock);
        bool cannot_pressurize_tank = FaultState::Propulsion::cannot_pressurize_outer_tank;
        bool cannot_pressurize_tank_ignored = FaultState::Propulsion::cannot_pressurize_outer_tank_ignored;
    rwMtxRUnlock(&FaultState::Propulsion::propulsion_faults_state_lock);
    if (cannot_pressurize_tank || !cannot_pressurize_tank_ignored)
        return true;
    
    // Check ADCS HAT
    // Check if all magnetometers are faulty
    bool all_magnetometers_faulty = true;
    bool all_magnetometers_faulty_ignore = State::read(FaultState::ADCS::all_magnetometers_faulty_ignore, 
                                                       FaultState::ADCS::adcs_faults_state_lock);
    rwMtxRLock(&State::ADCS::adcs_state_lock);
        all_magnetometers_faulty &= !State::ADCS::adcs_hat.at("magnetometer_1").is_functional;
        all_magnetometers_faulty &= !State::ADCS::adcs_hat.at("magnetometer_2").is_functional;
    rwMtxRUnlock(&State::ADCS::adcs_state_lock);
    if (all_magnetometers_faulty && !all_magnetometers_faulty_ignore) return true;

    // Check if any of the motors are faulty
    rwMtxRLock(&State::ADCS::adcs_state_lock);
        bool motor_x_faulty = !State::ADCS::adcs_hat.at("motor_x").is_functional;
        bool motor_y_faulty = !State::ADCS::adcs_hat.at("motor_y").is_functional;
        bool motor_z_faulty = !State::ADCS::adcs_hat.at("motor_z").is_functional;
    rwMtxRUnlock(&State::ADCS::adcs_state_lock);
    rwMtxRLock(&FaultState::ADCS::adcs_faults_state_lock);
        bool motor_x_faulty_ignore = FaultState::ADCS::motor_x_faulty_ignore;
        bool motor_y_faulty_ignore = FaultState::ADCS::motor_x_faulty_ignore;
        bool motor_z_faulty_ignore = FaultState::ADCS::motor_x_faulty_ignore;
    rwMtxRUnlock(&FaultState::ADCS::adcs_faults_state_lock);
    if (motor_x_faulty && !motor_x_faulty_ignore) return true;
    if (motor_y_faulty && !motor_y_faulty_ignore) return true;
    if (motor_z_faulty && !motor_z_faulty_ignore) return true;

    // Check if all sun sensors are faulty
    bool all_ssa_faulty = true;
    bool all_ssa_faulty_ignore = State::read(FaultState::ADCS::all_ssa_faulty_ignore, 
                                             FaultState::ADCS::adcs_faults_state_lock);
    rwMtxRLock(&State::ADCS::adcs_state_lock);
        all_ssa_faulty &= !State::ADCS::adcs_hat.at("ssa_adc_1").is_functional;
        all_ssa_faulty &= !State::ADCS::adcs_hat.at("ssa_adc_2").is_functional;
        all_ssa_faulty &= !State::ADCS::adcs_hat.at("ssa_adc_3").is_functional;
        all_ssa_faulty &= !State::ADCS::adcs_hat.at("ssa_adc_4").is_functional;
        all_ssa_faulty &= !State::ADCS::adcs_hat.at("ssa_adc_5").is_functional;
    rwMtxRUnlock(&State::ADCS::adcs_state_lock);
    if (all_ssa_faulty && !all_ssa_faulty_ignore) return true;

    return false;
}