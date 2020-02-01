#include "QuakeManager.h"
#include "Drivers/QLocate.hpp"

#include "radio_state_t.enum"

// Include I/O functions for telemetry dumping during functional testing.
#ifdef FUNCTIONAL_TEST
    #ifdef DESKTOP
        #include <iostream>
        #include <iomanip>
        #include <sstream>
    #else
        #include <Arduino.h>
    #endif
#endif

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
    TimedControlTask<bool>(registry, "quake", offset),
    max_wait_cycles_f("radio.max_wait", Serializer<unsigned int>(0, 24 * 60 * 60 * 1000 / 120)),
    max_transceive_cycles_f("radio.max_transceive", Serializer<unsigned int>(0, 24 * 60 * 60 * 1000 / 120)),
    radio_err_f("radio.err", Serializer<int>(-90, 10)),
    radio_mt_packet_f("uplink.ptr"),
    radio_mt_len_f("uplink.len"),
    radio_state_f("radio.state"),
    last_checkin_cycle_f("radio.last_comms_ccno"), // Last communication control cycle #
    dump_telemetry_f("telem.dump", Serializer<bool>()),
    qct(registry),
    mo_idx(0),
    unexpected_flag(false)
{ 
    add_writable_field(max_wait_cycles_f);
    add_writable_field(max_transceive_cycles_f);
    add_readable_field(radio_err_f);
    add_internal_field(radio_mt_packet_f);
    add_internal_field(radio_mt_len_f);
    add_internal_field(radio_state_f);
    add_internal_field(last_checkin_cycle_f);

    #ifdef FUNCTIONAL_TEST
    add_writable_field(dump_telemetry_f);
    #endif

    // Retrieve fields from registry
    snapshot_size_fp = find_internal_field<size_t>("downlink.snap_size", __FILE__, __LINE__);
    radio_mo_packet_fp = find_internal_field<char*>("downlink.ptr", __FILE__, __LINE__);

    // Initialize Quake Manager variables
    max_wait_cycles_f.set(1);
    max_transceive_cycles_f.set(500);
    last_checkin_cycle_f.set(control_cycle_count);
    qct.request_state(CONFIG);
    radio_mt_packet_f.set(qct.get_MT_msg());
    radio_mt_len_f.set(0);
    radio_state_f.set(static_cast<unsigned int>(radio_state_t::disabled));
    radio_state_f.set(static_cast<unsigned int>(radio_state_t::config));
    dump_telemetry_f.set(false);

    // Setup MO Buffers
    max_snapshot_size = std::max(snapshot_size_fp->get() + 1, static_cast<size_t>(packet_size));
    mo_buffer_copy = new char[max_snapshot_size];
}

QuakeManager::~QuakeManager()
{
    delete[] mo_buffer_copy;
}

bool QuakeManager::execute() {
    // printf(debug_severity::info, "[Quake Info] Executing Quake Manager "
    //     "current radio_state %d, current control task state %d", 
    //         radio_state_f.get(), 
    //         qct.get_current_state());

    #ifdef FUNCTIONAL_TEST
    if (dump_telemetry_f.get()) {
        dump_telemetry_f.set(false);
        char* snapshot = radio_mo_packet_fp->get();

        #ifdef DESKTOP
            std::cout << "{\"t\":" << debug_console::_get_elapsed_time() << ",\"telem\":\"";
            for(size_t i = 0; i < snapshot_size_fp->get(); i++) {
                std::ostringstream out;
                out << "\\\\x";
                out << std::hex << std::setfill('0') << std::setw(2) << (0xFF & snapshot[i]);
                std::cout << out.str();
            }
            std::cout << "\"}\n";
        #else
            Serial.printf("{\"t\":%d,\"telem\":\"", debug_console::_get_elapsed_time());
            for(size_t i = 0; i < snapshot_size_fp->get(); i++) {
                Serial.print("\\\\x");
                Serial.print((0xFF & snapshot[i]), HEX);
            }
            Serial.print("\"}\n");
        #endif
    }
    #endif

    const radio_state_t radio_state = static_cast<radio_state_t>(radio_state_f.get());
    switch(radio_state) {
        case radio_state_t::disabled:
        return true;
        case radio_state_t::config:
        return dispatch_config();
        case radio_state_t::wait:
        return dispatch_wait();
        case radio_state_t::transceive:
        return dispatch_transceive();
        case radio_state_t::read:
        return dispatch_read();
        case radio_state_t::write:
        return dispatch_write();
        default:
            printf(debug_severity::error, "Radio state not defined: %d", 
                radio_state_f.get());
            return false;
    }
}

bool QuakeManager::dispatch_config() {
    if (qct.get_current_state() != CONFIG) 
        return write_to_error(Devices::WRONG_STATE);
    if (unexpected_flag) return transition_radio_state(radio_state_t::wait);

    if ( no_more_cycles(max_config_cycles, radio_state_t::wait) )
    {
        unexpected_flag = true;
        return false;
    }

    int err_code = qct.execute();
    // If we have finished executing this command, then transition to waiting
    if ( qct.get_current_state() == IDLE )
        transition_radio_state(radio_state_t::write);
    return write_to_error(err_code);
}

bool QuakeManager::dispatch_wait() {
    // If we still have cycles, return true
    if (control_cycle_count - last_checkin_cycle_f.get() <= max_wait_cycles_f.get())
        return true;
    // Transition to config to attempt to resolve unexpected errors
    if (unexpected_flag) 
    {
        unexpected_flag = false; // clear the flag
        return transition_radio_state(radio_state_t::config); 
    }
    // Otherwise, transition to write in order to load new snapshot
    return transition_radio_state(radio_state_t::write);
}

bool QuakeManager::dispatch_write() {
    if (qct.get_current_state() != SBDWB) 
        return write_to_error(Devices::WRONG_STATE);
    if (unexpected_flag) return transition_radio_state(radio_state_t::wait);

    if ( no_more_cycles(max_write_cycles, radio_state_t::wait) ){
        unexpected_flag = true;
        return false;
    }

    // If we just entered write, copy the current message to our local buf
    if (qct.get_current_fn_number() == 0)
    {
        // If mo_idx is 0 --> copy current snapshot to local buf
        if (mo_idx == 0) {
            memset(mo_buffer_copy, 0, max_snapshot_size);
            memcpy(mo_buffer_copy, radio_mo_packet_fp->get(), max_snapshot_size);
        }
        // load the current 70 bytes of the buffer
       qct.set_downlink_msg(mo_buffer_copy + (packet_size*mo_idx), packet_size);
       assert(max_snapshot_size/packet_size != 0);
       mo_idx = (mo_idx + 1) % (max_snapshot_size/packet_size);
    }

    int err_code = qct.execute();

    // If we are done with loading messages --> try to transceive
    if (qct.get_current_state() == IDLE)
    {
        // printf(debug_severity::info, 
        //     "[Quake Info] SBDWB finished, transitioning to SBDIX");
        transition_radio_state(radio_state_t::transceive);
    }

    return write_to_error(err_code);
}

bool QuakeManager::dispatch_transceive() {
    if (qct.get_current_state() != SBDIX) 
        return write_to_error(Devices::WRONG_STATE);
    if (unexpected_flag) return transition_radio_state(radio_state_t::wait);

    // If we run out of cycles --> go back to waiting
    if (no_more_cycles(max_transceive_cycles_f.get(), radio_state_t::wait))
        return false;

    int err_code = qct.execute();

    // If we have finished executing SBDIX, then see if we have a message
    if ( qct.get_current_state() == IDLE )
    {
        // Case 1: We have no comms --> try again
        if (qct.get_MO_status() > 4)
        {
            // printf(debug_severity::info, 
            //     "[Quake Info] SBDIX finished, we have no comms. "
            //         "Error code: %d", qct.get_MO_status());
            // we should stay in transceive but do not update last_checkin
            qct.request_state(SBDIX);
            return write_to_error(err_code);
        }

        // Case 2: We have comms and we have message --> read message
        if (qct.get_MT_status() == 1) // SBD message successfully retrieved
        {
            // printf(debug_severity::info, 
            //     "[Quake Info] SBDIX finished, transitioning to SBDRB");
            transition_radio_state(radio_state_t::read);
        }
        // Case 3: We have comms and we have no message --> load next message
        else
        {
            // printf(debug_severity::info, 
            //     "[Quake Info] SBDIX finished, transitioning to SBDWB");
            transition_radio_state(radio_state_t::write);
        }
    }
    return write_to_error(err_code);
}

bool QuakeManager::dispatch_read() {
    if (qct.get_current_state() != SBDRB) 
        return write_to_error(Devices::WRONG_STATE);
    if (unexpected_flag) return transition_radio_state(radio_state_t::wait);

    if (no_more_cycles(max_read_cycles, radio_state_t::wait))
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

        radio_mt_len_f.set(qct.get_MT_length());

        transition_radio_state(radio_state_t::write);
    }
    return write_to_error(err_code);
}

bool QuakeManager::write_to_error(int err_code)
{
    // These are the only acceptable errors
    if ((err_code == Devices::OK) || (err_code == Devices::PORT_UNAVAILABLE))
        return true;

    // Something unexpected definitely happened
    radio_err_f.set(err_code);
    unexpected_flag = true;
    printf(debug_severity::error, 
        "[Quake Error] Execution failed at radio state %d, quake control state "
        "%d, and fn_number %d with error code %d", 
        radio_state_f.get(),
        qct.get_current_state(),
        qct.get_current_fn_number(), 
        (error));
    transition_radio_state(radio_state_t::wait);
    return false;
}

bool QuakeManager::no_more_cycles(size_t max_cycles, radio_state_t new_state)
{
    if (control_cycle_count - last_checkin_cycle_f.get() > max_cycles)
    {
        printf(debug_severity::notice, 
            "[Quake Notice] Radio State %d has ran out of cycles.", 
            radio_state_f.get());
        // Transition to new_state
        transition_radio_state(new_state);
        return true;
    }
    return false;
}

bool QuakeManager::transition_radio_state(radio_state_t new_state)
{
    // printf(debug_severity::info, 
    //     "[Quake Info] Transitioning from radio state %d to %d", 
    //     static_cast<unsigned int> (radio_state_f),
    //     new_state);
    bool bOk = true;
    switch(new_state)
    {
        case radio_state_t::wait:
            bOk = qct.request_state(IDLE);
            break;
        case radio_state_t::config:
            bOk = qct.request_state(CONFIG);
            break;
        case radio_state_t::read:
            bOk = qct.request_state(SBDRB);
            break;
        case radio_state_t::write:
            bOk = qct.request_state(SBDWB);
            break;
        case radio_state_t::transceive:
            bOk = qct.request_state(SBDIX);
            break;
        default:
        printf(debug_severity::error, "In transition_radio_state:: Radio state not defined: %d", 
            radio_state_f.get());
    }
    // Update the last checkin cycle
    last_checkin_cycle_f.set(control_cycle_count);
    radio_state_f.set(static_cast<unsigned int>(new_state));

    if ( !bOk ) // Sanity check
        printf(debug_severity::error, "Invalid state transition from %d to %d",
        qct.get_current_state(),
        radio_state_f.get());
    return bOk;
}
