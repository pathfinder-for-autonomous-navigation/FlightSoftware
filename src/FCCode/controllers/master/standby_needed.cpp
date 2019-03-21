#include "master_helpers.hpp"
#include "../../state/device_states.hpp"
#include "../../state/fault_state_holder.hpp"

using State::Master::master_state_lock;

bool Master::standby_needed() {
    if (!State::Hardware::is_functional(&Devices::piksi())) return true;
    if (!State::Hardware::is_functional(&Devices::quake())) return true;
    
    // Check comms timeout
    State::Master::PANState pan_state = State::read(State::Master::pan_state, master_state_lock);
    bool is_close_approach = (pan_state == State::Master::PANState::FOLLOWER_CLOSE_APPROACH) || 
                             (pan_state == State::Master::PANState::LEADER_CLOSE_APPROACH);
    unsigned int uplink_timeout = Constants::read(Constants::Quake::UPLINK_TIMEOUT);
    if (State::Quake::msec_since_last_sbdix() >= uplink_timeout && !is_close_approach) {
        return true;
    }

    // Check if CDGPS lock is not obtained while satellites <100 m apart
    bool is_fixed_rtk = State::read(State::Piksi::is_fixed_rtk, State::Piksi::piksi_state_lock);
    bool is_float_rtk = State::read(State::Piksi::is_float_rtk, State::Piksi::piksi_state_lock);
    bool is_rtk = is_fixed_rtk || is_float_rtk;
    if (State::GNC::distance() < Constants::Piksi::CDGPS_RANGE && !is_rtk) {
        return true;
    }

    // Check if propulsion issues happened
    if (!State::read(FaultState::Propulsion::destabilization_event.is_not_set, 
            FaultState::Propulsion::propulsion_faults_state_lock))
        return true;
    if (!State::read(FaultState::Propulsion::overpressure_event.is_not_set, 
            FaultState::Propulsion::propulsion_faults_state_lock))
        return true;

    return false;
}