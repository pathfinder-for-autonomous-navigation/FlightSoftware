
#include "QuakeManager.h"
#include "QLocate.hpp"

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
    qct(registry, offset)
{ 
    // Retrieve fields from registry
    control_cycle_count_fp = find_readable_field<unsigned int>("pan.cycle_no", __FILE__, __LINE__);
    snapshot_size_fp = find_internal_field<unsigned int>("downlink_producer.snap_size", __FILE__, __LINE__);
    radio_mo_packet_fp = find_internal_field<char*>("downlink_producer.mo_ptr", __FILE__, __LINE__);
    radio_mt_packet_fp = find_internal_field<char*>("downlink_producer.mt_ptr", __FILE__, __LINE__);
    radio_err_fp = find_internal_field<int>("downlink_producer.radio_err_ptr", __FILE__, __LINE__);

    // Initialize Quake Manager variables
    last_checkin_cycle = control_cycle_count_fp->get();
    qct.request_state(CONFIG);
    unexpectedFlag = false;

    // Setup MO Buffers
    max_snapshot_size = snapshot_size_fp->get();
    mo_buffer_copy = new char[max_snapshot_size];
}

QuakeManager::~QuakeManager()
{
    delete [] mo_buffer_copy;
}

bool QuakeManager::execute() {
    printf(debug_severity::info, "[Quake Info] Executing Quake Manager \
        current radio_state %d, current control task state %d\r\n", 
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
            printf(debug_severity::error, "Radio state not defined: %d\r\n", 
            static_cast<unsigned int>(radio_mode_f));
            return false;
    }
}

bool QuakeManager::dispatch_config() {
    if (qct.get_current_state() != CONFIG) 
        return write_to_error(Devices::WRONG_STATE);

    if ( no_more_cycles(
            max_config_cycles, 
            radio_mode_t::wait) )
        return false;

    int err_code = qct.execute();
    // If we have finished executing this command, then transition to waiting
    if ( qct.get_current_state() == IDLE )
        transition_radio_state(radio_mode_t::wait);
    return write_to_error(err_code);
}

bool QuakeManager::dispatch_wait() {
    // If we are done waiting --> load a message
    if (unexpectedFlag) 
    {
        transition_radio_state(radio_mode_t::config);
    }
        
    if ( no_more_cycles(
            max_wait_cycles, 
            radio_mode_t::write) )
        return false;
    return true;
}

bool QuakeManager::dispatch_transceive() {
    if (qct.get_current_state() != SBDIX) 
        return write_to_error(Devices::WRONG_STATE);

    // If we run out of cycles --> go back to waiting
    if ( no_more_cycles(
            max_transceive_cycles, 
            radio_mode_t::wait) )
        return false;

    // If we have first entered this state, copy a new_snapshot
    if (qct.get_current_fn_number() == 0) 
    {
        mo_idx = 0;
        update_mo();
    }
    int err_code = qct.execute();

    // If we have finished executing SBDIX, then see if we have a message
    if ( qct.get_current_state() == IDLE )
    {
        // Case 1: We have no comms --> try again
        if (qct.quake.sbdix_r[0] > 4)
        {
            printf(debug_severity::info, 
                "[Quake Info] SBDIX finished, we have no comms. \
                    Error code: %d \r\n", qct.quake.sbdix_r[0]);
            return write_to_error(err_code);
        }

        // Case 2: We have comms and we have message --> read message
        if (qct.quake.sbdix_r[4] > 0) 
        {
            printf(debug_severity::info, 
                "[Quake Info] SBDIX finished, transitioning to SBDRB\r\n");
            qct.request_state(SBDRB);
            transition_radio_state(radio_mode_t::read);
        }
        // Case 3: We have comms and we have no message --> load next message
        else
        {
            printf(debug_severity::info, 
                "[Quake Info] SBDIX finished, transitioning to SBDWB\r\n");
            qct.request_state(SBDWB);
            transition_radio_state(radio_mode_t::write);
        }
    }
    return write_to_error(err_code);
}

bool QuakeManager::dispatch_read() {
    if (qct.get_current_state() != SBDRB) 
        return write_to_error(Devices::WRONG_STATE);

    if ( no_more_cycles(
            max_read_cycles, 
            radio_mode_t::wait) )
        return false;

    int err_code = qct.execute();

    // If we are done with SBDRB --> save message and load next message
    if (qct.get_current_state() == IDLE)
    {
        printf(debug_severity::info, 
            "[Quake Info] SBDRB finished, transitioning to SBDWB\r\n");

        radio_mt_packet_fp->set(qct.quake.get_message());
        qct.request_state(SBDWB);
        transition_radio_state(radio_mode_t::write);
    }
    return write_to_error(err_code);
}

bool QuakeManager::dispatch_write() {
    if (qct.get_current_state() != SBDWB) 
        return write_to_error(Devices::WRONG_STATE);

    if ( no_more_cycles(
            max_write_cycles, 
            radio_mode_t::wait) )
        return false;

    // If we just entered this state --> load current snapshot
    if (qct.get_current_fn_number() == 0)
    {
       qct.set_downlink_msg(mo_buffer_copy + (packet_size*mo_idx), packet_size);
       update_mo();
    }

    int err_code = qct.execute();

    // If we are done with loading messages --> try to transceive
    if (qct.get_current_state() == IDLE)
    {
        printf(debug_severity::info, 
            "[Quake Info] SBDWB finished, transitioning to SBDIX\r\n");
        qct.request_state(SBDIX);
        transition_radio_state(radio_mode_t::transceive);
    }

    return write_to_error(err_code);
}

bool QuakeManager::dispatch_manual(){
    // In this mode keep checking for comms
    return false;
}

bool QuakeManager::write_to_error(int err_code)
{
    if (err_code == Devices::OK) return true;

    radio_err_fp->set(err_code);

    printf(debug_severity::error, 
        "[Quake Error] Execution failed at radio state %d, quake control state \
        %d, and fn_number %d with error code %d\r\n", 
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
            "[Quake Notice] Radio State %d has ran out of cycles.\r\n", 
            static_cast<unsigned int> (radio_mode_f));
        // Transition to new_state
        transition_radio_state(new_state);
        // Update QCT state to IDLE
        qct.request_state(IDLE); 
        return false;
    }
    return true;
}

void QuakeManager::transition_radio_state(radio_mode_t new_state)
{
    printf(debug_severity::info, 
        "[Quake Info] Transitioning from radio state %d to %d\r\n", 
        static_cast<unsigned int> (radio_mode_f),
        new_state);

    // Update the last checkin cycle
    last_checkin_cycle = control_cycle_count_fp->get();
    radio_mode_f = new_state;
}

void QuakeManager::update_mo()
{
    if (mo_idx == 0)
    {
        printf(debug_severity::info, "[Quake Info] Updating MO buffer...\
        copying new snapshot\r\n");
        memcpy(mo_buffer_copy, radio_mo_packet_fp->get(), max_snapshot_size);
    }
    else
    {
        printf(debug_severity::info, "[Quake Info] Updating MO buffer...\
        incrementing to the next 70 bytes\r\n");
        mo_idx = (mo_idx + 1) % (max_snapshot_size/packet_size);
    }
}