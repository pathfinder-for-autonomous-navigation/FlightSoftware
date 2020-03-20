#include "SimpleFaultHandler.hpp"

SimpleFaultHandler::SimpleFaultHandler(StateFieldRegistry& r, Fault* f,
    const std::vector<mission_state_t>& _active_states,
    mission_state_t rs) :
        FaultHandlerMachine(r),
        fault(f),
        active_states(_active_states),
        recommended_state(rs)
{
    assert(rs == mission_state_t::safehold || rs == mission_state_t::standby);
}

fault_response_t SimpleFaultHandler::determine_recommended_state() const {
    const mission_state_t state = static_cast<mission_state_t>(mission_state_fp->get());
    if (std::find(active_states.begin(), active_states.end(), state) == active_states.end())
        return fault_response_t::none;

    if (fault->is_faulted()) {
        if (recommended_state == mission_state_t::standby) {
            return fault_response_t::standby;
        }
        else {
            return fault_response_t::safehold;
        }
    }
    else return fault_response_t::none;
}

const std::vector<std::vector<mission_state_t>> SimpleFaultHandler::active_state_lists {
    // List 0 is used by the Gomspace's low-battery fault
    {
        mission_state_t::initialization_hold,
        mission_state_t::detumble,
        mission_state_t::standby,
        mission_state_t::follower,
        mission_state_t::leader,
        mission_state_t::follower_close_approach,
        mission_state_t::leader_close_approach
    },
    // List 1 is used by the ADCS and propulsion faults
    {
        mission_state_t::standby,
        mission_state_t::follower,
        mission_state_t::leader,
        mission_state_t::follower_close_approach,
        mission_state_t::leader_close_approach
    }
};

void SimpleFaultHandler::set_mission_state_ptr(WritableStateField<unsigned char>* ptr) {
    mission_state_fp = ptr;
}

const WritableStateField<unsigned char>* SimpleFaultHandler::mission_state_fp = nullptr;

SuperSimpleFaultHandler::SuperSimpleFaultHandler(StateFieldRegistry& r, Fault* f,
        const std::vector<mission_state_t>& _active_states,
        mission_state_t rs) : SimpleFaultHandler(r, f, _active_states, rs) {}

fault_response_t SuperSimpleFaultHandler::execute() {
    return determine_recommended_state();
}
