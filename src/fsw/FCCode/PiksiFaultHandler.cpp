#include "PiksiFaultHandler.hpp"
#include "constants.hpp"

// Declare static storage for constexpr variables
const constexpr unsigned int PiksiFaultHandler::default_no_cdgps_max_wait;
const constexpr unsigned int PiksiFaultHandler::default_cdgps_delay_max_wait;
const unsigned int &count_cc = TimedControlTaskBase::control_cycle_count;

PiksiFaultHandler::PiksiFaultHandler(StateFieldRegistry& r) 
    : FaultHandlerMachine(r), 
    no_cdgps_max_wait_f("piksi_fh.no_cdpgs_max_wait", Serializer<unsigned int>(PAN::one_day_ccno)),
    cdgps_delay_max_wait_f("piksi_fh.cdpgs_delay_max_wait", Serializer<unsigned int>(PAN::one_day_ccno)),
    fault_handler_enabled_f("piksi_fh.enabled", Serializer<bool>()),
    last_rtkfix_ccno_f("piksi_fh.last_rtkfix_ccno"),
    piksi_dead_fault_f("piksi_fh.dead", piksi_dead_threshold)
    {
        add_writable_field(no_cdgps_max_wait_f);
        add_writable_field(cdgps_delay_max_wait_f);
        add_writable_field(fault_handler_enabled_f);
        add_internal_field(last_rtkfix_ccno_f);
        add_fault(piksi_dead_fault_f);

        // Initialize to 24 hours
        no_cdgps_max_wait_f.set(default_no_cdgps_max_wait);
        // Initialize to 3 hours
        cdgps_delay_max_wait_f.set(default_cdgps_delay_max_wait);
        // Default enable to true
        fault_handler_enabled_f.set(true);

        piksi_state_fp = find_readable_field<unsigned char>("piksi.state", __FILE__, __LINE__);
        mission_state_fp = find_writable_field<unsigned char>("pan.state", __FILE__, __LINE__);
        last_rtkfix_ccno_f.set(0);
        enter_close_appr_time_fp = find_internal_field<unsigned int>("pan.enter_close_approach_ccno", __FILE__, __LINE__);
    }

fault_response_t PiksiFaultHandler::execute() {
    if (!fault_handler_enabled_f.get()) return fault_response_t::none;

    piksi_mode_t piksi_state = static_cast<piksi_mode_t>(piksi_state_fp->get());
    mission_state_t mission_state = static_cast<mission_state_t>(mission_state_fp->get());

    // piksi_dead_fault control section
    if (mission_state != mission_state_t::startup 
     && mission_state != mission_state_t::detumble
     && mission_state != mission_state_t::initialization_hold
     && mission_state != mission_state_t::standby
    
    && (piksi_state == piksi_mode_t::crc_error
     || piksi_state == piksi_mode_t::no_data_error 
     || piksi_state == piksi_mode_t::data_error))
        piksi_dead_fault_f.signal();
    else
        piksi_dead_fault_f.unsignal();

    if(piksi_dead_fault_f.is_faulted())
        return fault_response_t::standby;

    // begin rtk section

    if (piksi_state == piksi_mode_t::fixed_rtk) {
        last_rtkfix_ccno_f.set(count_cc);
    }

    if (mission_state == mission_state_t::follower_close_approach || 
            mission_state == mission_state_t::leader_close_approach) {
        return check_cdgps();
    }
    
    return fault_response_t::none;
}

fault_response_t PiksiFaultHandler::check_cdgps() {
    unsigned int close_appr_time = enter_close_appr_time_fp->get();
    unsigned int last_rtkfix_time = last_rtkfix_ccno_f.get();
    unsigned int duration = TimedControlTaskBase::control_cycle_count-std::max(close_appr_time ,last_rtkfix_time);

    // Recommend moving to standby if we haven't recieved any readings in X time since 
    // moving to close approach state
    if (close_appr_time >= last_rtkfix_time && duration > no_cdgps_max_wait_f.get()) {
        return fault_response_t::standby;
    }
    
    // If we have recieved CDGPS readings since entering close approach state,
    // then recommend moving to standby if we haven't recieved any more readings 
    // in Y time since the last reading in close approach
    if (last_rtkfix_time > close_appr_time && duration > cdgps_delay_max_wait_f.get()) {
        return fault_response_t::standby;
    }

    return fault_response_t::none;
}