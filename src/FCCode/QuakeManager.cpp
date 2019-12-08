#include "QuakeManager.h"
#include "QLocate.hpp"
#define RADIO_TESTING
/**
 * QuakeManager Implementation Info: 
 * 
 * QuakeManager is implemented as a state machine. 
 * Transitions occur when a state has exhausted its allocated cycles or 
 * the QuakeControlTask is in the IDLE state since that indicates that it 
 * has finished its current task. 
 * 
 * Dependencies: QuakeControlTask, Devices
 * 
 */

// Quake driver setup is initialized when QuakeController constructor is called
QuakeManager::QuakeManager(StateFieldRegistry &registry, unsigned int offset) : 
    TimedControlTask<bool>(registry, offset),
    radio_mode_f(radio_mode_t::config),
    qct(registry),
    mo_idx(0),
    unexpected_flag(false)
{ 
    // Retrieve fields from registry
    control_cycle_count_fp = find_readable_field<unsigned int>("pan.cycle_no", __FILE__, __LINE__);
    snapshot_size_fp = find_internal_field<size_t>("downlink_producer.snap_size", __FILE__, __LINE__);
    radio_mo_packet_fp = find_internal_field<char*>("downlink_producer.mo_ptr", __FILE__, __LINE__);
    radio_mt_packet_fp = find_internal_field<char*>("uplink_consumer.mt_ptr", __FILE__, __LINE__);
    radio_err_fp = find_readable_field<int>("downlink_producer.radio_err_ptr", __FILE__, __LINE__);
    radio_mt_ready_fp = find_internal_field<bool>("uplink_consumer.mt_ready", __FILE__, __LINE__);

    assert(control_cycle_count_fp);
    assert(snapshot_size_fp);
    assert(radio_mo_packet_fp);
    assert(radio_mt_packet_fp);
    assert(radio_err_fp);
    assert(radio_mt_ready_fp);

    // Initialize Quake Manager variables
    last_checkin_cycle = control_cycle_count_fp->get();
    qct.request_state(CONFIG);
    radio_mt_packet_fp->set(qct.get_MT_msg());
    radio_mt_ready_fp->set(false);

#ifdef RADIO_TESTING
    max_snapshot_size = 145;
    mo_buffer_copy = new char[max_snapshot_size];
#else
    // Setup MO Buffers
    max_snapshot_size = std::max(snapshot_size_fp->get() + 1, static_cast<size_t>(packet_size));
    mo_buffer_copy = new char[max_snapshot_size];
#endif
}

QuakeManager::~QuakeManager()
{
    delete[] mo_buffer_copy;
}

bool QuakeManager::execute() {
    printf(debug_severity::info, "[Quake Info] Executing Quake Manager \
        current radio_state %d, current control task state %d", 
            static_cast<unsigned int>(radio_mode_f), 
            qct.get_current_state());
    switch(radio_mode_f){
        case radio_mode_t::config:
        return dispatch_config();
        case radio_mode_t::wait:
        return dispatch_wait();
        case radio_mode_t::transceive:
        return dispatch_transceive();
        case radio_mode_t::read:
        return dispatch_read();
        case radio_mode_t::write:
        return dispatch_write();
        case radio_mode_t::manual:
        return dispatch_manual();
        default:
            printf(debug_severity::error, "Radio state not defined: %d", 
            static_cast<unsigned int>(radio_mode_f));
            return false;
    }
}

bool QuakeManager::dispatch_config() {
    if (qct.get_current_state() != CONFIG) 
        return write_to_error(Devices::WRONG_STATE);
    if (unexpected_flag) return transition_radio_state(radio_mode_t::wait);

    if ( no_more_cycles(max_config_cycles, radio_mode_t::wait) )
    {
        unexpected_flag = true;
        return false;
    }

    int err_code = qct.execute();
    // If we have finished executing this command, then transition to waiting
    if ( qct.get_current_state() == IDLE )
        transition_radio_state(radio_mode_t::write);
    return write_to_error(err_code);
}

bool QuakeManager::dispatch_wait() {
    // If we still have cycles, return true
    if (control_cycle_count_fp->get() - last_checkin_cycle <= max_wait_cycles)
        return true;
    // Transition to config to attempt to resolve unexpected errors
    if (unexpected_flag) 
    {
        unexpected_flag = false; // clear the flag
        return transition_radio_state(radio_mode_t::config); 
    }
    // Otherwise, transition to write in order to load new snapshot
    return transition_radio_state(radio_mode_t::write);
}

bool QuakeManager::dispatch_write() {
    if (qct.get_current_state() != SBDWB) 
        return write_to_error(Devices::WRONG_STATE);
    if (unexpected_flag) return transition_radio_state(radio_mode_t::wait);

    if ( no_more_cycles(max_write_cycles, radio_mode_t::wait) ){
        unexpected_flag = true;
        return false;
    }

    // If we just entered write, copy the current message to our local buf
    if (qct.get_current_fn_number() == 0)
    {
        // If mo_idx is 0 --> copy current snapshot to local buf
#ifdef RADIO_TESTING
        std::string json_string = "{\"arr\":[1,2,3],\"boolean\": true,\"foo\": 77777.655432,\"null\": null,\"number\": 123,\"object\": {\"a\": \"b\",\"c\": \"d\", \"e\": \"f\"},\"string\": \"Hello World\"}";
        if (mo_idx == 0) {
            memset(mo_buffer_copy, 0, 145);
            memcpy(mo_buffer_copy, json_string.c_str(), 145);
        }
#else
        if (mo_idx == 0) {
            memset(mo_buffer_copy, 0, max_snapshot_size);
            memcpy(mo_buffer_copy, radio_mo_packet_fp->get(), max_snapshot_size);
        }
#endif
        // load the current 70 bytes of the buffer
       qct.set_downlink_msg(mo_buffer_copy + (packet_size*mo_idx), packet_size);
       mo_idx = (mo_idx + 1) % (max_snapshot_size/packet_size);
    }

    int err_code = qct.execute();

    // If we are done with loading messages --> try to transceive
    if (qct.get_current_state() == IDLE)
    {
        printf(debug_severity::info, 
            "[Quake Info] SBDWB finished, transitioning to SBDIX");
        transition_radio_state(radio_mode_t::transceive);
    }

    return write_to_error(err_code);
}

bool QuakeManager::dispatch_transceive() {
    if (qct.get_current_state() != SBDIX) 
        return write_to_error(Devices::WRONG_STATE);
    if (unexpected_flag) return transition_radio_state(radio_mode_t::wait);

    // If we run out of cycles --> go back to waiting
    if (no_more_cycles(max_transceive_cycles, radio_mode_t::wait))
        return false;

    int err_code = qct.execute();

    // If we have finished executing SBDIX, then see if we have a message
    if ( qct.get_current_state() == IDLE )
    {
        // Case 1: We have no comms --> try again
        if (qct.get_MO_status() > 4)
        {
            printf(debug_severity::info, 
                "[Quake Info] SBDIX finished, we have no comms. \
                    Error code: %d", qct.get_MO_status());
            // we should stay in transceive but do not update last_checkin
            qct.request_state(SBDIX);
            return write_to_error(err_code);
        }

        // Case 2: We have comms and we have message --> read message
        if (qct.get_MT_status() == 1) // SBD message successfully retrieved
        {
            printf(debug_severity::info, 
                "[Quake Info] SBDIX finished, transitioning to SBDRB");
            transition_radio_state(radio_mode_t::read);
        }
        // Case 3: We have comms and we have no message --> load next message
        else
        {
            printf(debug_severity::info, 
                "[Quake Info] SBDIX finished, transitioning to SBDWB");
            transition_radio_state(radio_mode_t::write);
        }
    }
    return write_to_error(err_code);
}

bool QuakeManager::dispatch_read() {
    if (qct.get_current_state() != SBDRB) 
        return write_to_error(Devices::WRONG_STATE);
    if (unexpected_flag) return transition_radio_state(radio_mode_t::wait);

    if (no_more_cycles(max_read_cycles, radio_mode_t::wait))
    {
        unexpected_flag = true;
        return false;
    }

    int err_code = qct.execute();

    // If we are done with SBDRB --> save message and load next message
    if (qct.get_current_state() == IDLE)
    {
        printf(debug_severity::info, 
            "[Quake Info] SBDRB finished, transitioning to SBDWB");

        radio_mt_ready_fp->set(true);
        transition_radio_state(radio_mode_t::write);
    }
    return write_to_error(err_code);
}

bool QuakeManager::dispatch_manual(){
    // In this mode keep checking for comms
    return false;
}

bool QuakeManager::write_to_error(int err_code)
{
    // These are the only acceptable errors
    if ((err_code == Devices::OK) || (err_code == Devices::PORT_UNAVAILABLE))
        return true;

    // Something unexpected definitely happened
    radio_err_fp->set(err_code);
    unexpected_flag = true;
    printf(debug_severity::error, 
        "[Quake Error] Execution failed at radio state %d, quake control state \
        %d, and fn_number %d with error code %d", 
        static_cast<unsigned int> (radio_mode_f),
        qct.get_current_state(),
        qct.get_current_fn_number(), 
        (error));
    transition_radio_state(radio_mode_t::wait);
    return false;
}

bool QuakeManager::no_more_cycles(size_t max_cycles, radio_mode_t new_state)
{
    if (control_cycle_count_fp->get() - last_checkin_cycle > max_cycles)
    {
        printf(debug_severity::notice, 
            "[Quake Notice] Radio State %d has ran out of cycles.", 
            static_cast<unsigned int> (radio_mode_f));
        // Transition to new_state
        transition_radio_state(new_state);
        return true;
    }
    return false;
}

bool QuakeManager::transition_radio_state(radio_mode_t new_state)
{
    printf(debug_severity::info, 
        "[Quake Info] Transitioning from radio state %d to %d", 
        static_cast<unsigned int> (radio_mode_f),
        new_state);
    bool bOk = true;
    switch(new_state)
    {
        case radio_mode_t::wait:
            bOk = qct.request_state(IDLE);
            break;
        case radio_mode_t::config:
            bOk = qct.request_state(CONFIG);
            break;
        case radio_mode_t::read:
            bOk = qct.request_state(SBDRB);
            break;
        case radio_mode_t::write:
            bOk = qct.request_state(SBDWB);
            break;
        case radio_mode_t::transceive:
            bOk = qct.request_state(SBDIX);
            break;
        default:
        printf(debug_severity::error, "In transition_radio_state:: Radio state not defined: %d", 
            static_cast<unsigned int>(radio_mode_f));
    }
    // Update the last checkin cycle
    last_checkin_cycle = control_cycle_count_fp->get();
    radio_mode_f = new_state;

    if ( !bOk ) // Sanity check
        printf(debug_severity::error, "Invalid state transition from %d to %d",
        qct.get_current_state(),
        static_cast<unsigned int>(radio_mode_f));
    return bOk;
}
