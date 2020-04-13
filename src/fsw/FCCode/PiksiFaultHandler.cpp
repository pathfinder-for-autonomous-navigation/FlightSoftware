#include "PiksiFaultHandler.hpp"
#include "constants.hpp"

const unsigned int& control_cycle_count = TimedControlTaskBase::control_cycle_count;

PiksiFaultHandler::PiksiFaultHandler(StateFieldRegistry& r) : FaultHandlerMachine(r) {
    piksi_state_fp = find_readable_field<unsigned int>("piksi.state", __FILE__, __LINE__);
    mission_state_fp = find_writable_field<unsigned char>("pan.state", __FILE__, __LINE__);
    last_fix_time_fp  = find_internal_field<sys_time_t>("piksi.last_fix_time", __FILE__, __LINE__);
}

fault_response_t PiksiFaultHandler::execute() {
    switch(current_state) {
        case piksi_fh_state_t::unfaulted:      return dispatch_unfaulted();   break;
        case piksi_fh_state_t::check_cdgps:    return dispatch_check_cdgps();     break;
        case piksi_fh_state_t::no_cdgps:       return dispatch_no_cdgps();    break;
        case piksi_fh_state_t::cdgps_delay:    return dispatch_cdgps_delay(); break;
        default:
            current_state = piksi_fh_state_t::unfaulted;
            return fault_response_t::none;
    }
    return fault_response_t::none;
}

void PiksiFaultHandler::transition_to(piksi_fh_state_t next_state) {
    current_state = next_state;
    current_state_entry_ccno = control_cycle_count;
}

fault_response_t PiksiFaultHandler::dispatch_unfaulted() {
    if (piksi_state_fp->get() == piksi_mode_t::dead) {
        return fault_response_t::standby;
    }

    if (mission_state_fp->get() == mission_state_t::follower_close_approach || 
            mission_state_fp->get() == mission_state_t::leader_close_approach) {
        transition_to(piksi_fh_state_t::check_cdgps);
    }
    else {
        return fault_response_t::none;
    }
}

fault_response_t PiksiFaultHandler::dispatch_check_cdgps() {
    if (enter_close_appr_time_fp->get() > last_fix_time_fp->get()) {
        current_state = piksi_fh_state_t::no_cdgps;
    }
    else {
        current_state = piksi_fh_state_t::cdgps_delay;
    }
}

fault_response_t PiksiFaultHandler::dispatch_no_cdgps() {
    // I need the time we last last got a fix and the time we last entered close approach
    // systime_duration_t time_difference = get_time_difference(enter_close_appr_time_fp->get(), last_fix_time_fp->get());
}

fault_response_t PiksiFaultHandler::dispatch_cdgps_delay() {

}

systime_duration_t PiksiFaultHandler::get_time_duration(sys_time_t time1, sys_time_t time2) {
    #ifdef DESKTOP
    
    #else
    return std::abs(time2-time1);
    #endif
}