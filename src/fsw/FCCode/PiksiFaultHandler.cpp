#include "PiksiFaultHandler.hpp"
#include "constants.hpp"

const unsigned int& control_cycle_count = TimedControlTaskBase::control_cycle_count;

PiksiFaultHandler::PiksiFaultHandler(StateFieldRegistry& r) : FaultHandlerMachine(r) {
    piksi_state_fp = find_readable_field<unsigned int>("piksi.state", __FILE__, __LINE__);
    mission_state_fp = find_writable_field<unsigned char>("pan.state", __FILE__, __LINE__);
    last_fix_time_fp  = find_internal_field<sys_time_t>("piksi.last_fix_time", __FILE__, __LINE__);
}

fault_response_t PiksiFaultHandler::execute() {
    if (piksi_state_fp->get() == piksi_mode_t::dead) {
        return fault_response_t::standby;
    }

    if (mission_state_fp->get() == mission_state_t::follower_close_approach || 
            mission_state_fp->get() == mission_state_t::leader_close_approach) {
        return check_cdgps();
    }
    
    return fault_response_t::none;
}

fault_response_t PiksiFaultHandler::check_cdgps() {
    sys_time_t close_appr_time = enter_close_appr_time_fp->get();
    sys_time_t last_fix_time = last_fix_time_fp->get();
    systime_duration_t duration = TimedControlTaskBase::us_to_duration(close_appr_time-last_fix_time);

    if (close_appr_time > last_fix_time && duration > x) {
        return fault_response_t::standby;
    }
    else if (close_appr_time < last_fix_time && duration > y) {
        return fault_response_t::standby;
    }
    return fault_response_t::none;
}