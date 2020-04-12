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
        case piksi_fh_state_t::no_cdgps:       return dispatch_no_cdgps();    break;
        case piksi_fh_state_t::cdgps_delay:    return dispatch_cdgps_delay(); break;
        default:
            current_state = piksi_fh_state_t::unfaulted;
            return fault_response_t::none;
    }
    return fault_response_t::none;
}

fault_response_t PiksiFaultHandler::dispatch_unfaulted() {

}

fault_response_t PiksiFaultHandler::dispatch_no_cdgps() {

}

fault_response_t PiksiFaultHandler::dispatch_cdgps_delay() {

}