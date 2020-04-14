#include "PiksiFaultHandler.hpp"
#include "constants.hpp"
#include <iostream>

PiksiFaultHandler::PiksiFaultHandler(StateFieldRegistry& r) 
    : FaultHandlerMachine(r), 
    no_cdgps_max_wait_f("piksi_fh.no_cdpgs_max_wait", Serializer<unsigned int>(PAN::one_day_ccno)),
    cdgps_delay_max_wait_f("piksi_fh.cdpgs_delay_max_wait", Serializer<unsigned int>(PAN::one_day_ccno))
    {
        add_writable_field(no_cdgps_max_wait_f);
        add_writable_field(cdgps_delay_max_wait_f);

        // Initialize to 24 hours
        no_cdgps_max_wait_f.set(PAN::one_day_ccno);
        // Initialize to 3 hours
        cdgps_delay_max_wait_f.set(PAN::one_day_ccno/8);

        piksi_state_fp = find_readable_field<unsigned int>("piksi.state", __FILE__, __LINE__);
        mission_state_fp = find_writable_field<unsigned char>("pan.state", __FILE__, __LINE__);
        last_fix_ccno_fp  = find_internal_field<unsigned int>("piksi.last_fix_ccno", __FILE__, __LINE__);
        enter_close_appr_time_fp = find_internal_field<unsigned int>("pan.enter_close_approach_ccno", __FILE__, __LINE__);
    }

fault_response_t PiksiFaultHandler::execute() {
    piksi_mode_t piksi_state = static_cast<piksi_mode_t>(piksi_state_fp->get());
    mission_state_t mission_state = static_cast<mission_state_t>(mission_state_fp->get());

    if (piksi_state == piksi_mode_t::dead) {
        return fault_response_t::standby;
    }

    if (mission_state == mission_state_t::follower_close_approach || 
            mission_state == mission_state_t::leader_close_approach) {
        return check_cdgps();
    }
    
    return fault_response_t::none;
}

fault_response_t PiksiFaultHandler::check_cdgps() {
    unsigned int close_appr_time = enter_close_appr_time_fp->get();
    unsigned int last_fix_time = last_fix_ccno_fp->get();
    unsigned int duration = TimedControlTaskBase::control_cycle_count-std::max(close_appr_time ,last_fix_time);

    // Recommend moving to standby if we haven't recieved any readings in X time since 
    // moving to close approach state
    if (close_appr_time > last_fix_time && duration > no_cdgps_max_wait_f.get()) {
        return fault_response_t::standby;
    }
    
    // If we have recieved CDGPS readings since entering close approach state,
    // then recommend moving to standby if we haven't recieved any more readings 
    // in Y time since the last reading in close approach
    if (last_fix_time > close_appr_time && duration > cdgps_delay_max_wait_f.get()) {
        return fault_response_t::standby;
    }

    return fault_response_t::none;
}