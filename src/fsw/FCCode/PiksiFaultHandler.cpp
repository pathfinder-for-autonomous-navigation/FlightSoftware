#include "PiksiFaultHandler.hpp"
#include "constants.hpp"

const unsigned int& control_cycle_count = TimedControlTaskBase::control_cycle_count;

PiksiFaultHandler::PiksiFaultHandler(StateFieldRegistry& r) 
    : FaultHandlerMachine(r), 
    no_cdgps_max_wait_f("piksi_fh.no_cdpgs_max_wait", Serializer<unsigned int>(0,10e10, 10e10)),
    cdgps_delay_max_wait_f("piksi_fh.cdpgs_delay_max_wait", Serializer<unsigned int>(0,10e10, 10e10))
    {
        add_writable_field(no_cdgps_max_wait_f);
        add_writable_field(cdgps_delay_max_wait_f);

        // Initialize to 24 hours (8.64e10 micros)
        no_cdgps_max_wait_f.set(8.64e10);
        // Initialize to 3 hours (1.08e+10 micros)
        cdgps_delay_max_wait_f.set(1.08e+10);

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

    if (close_appr_time > last_fix_time && duration > no_cdgps_max_wait_f.get()) {
        return fault_response_t::standby;
    }
    else if (close_appr_time < last_fix_time && duration > cdgps_delay_max_wait_f.get()) {
        return fault_response_t::standby;
    }
    return fault_response_t::none;
}