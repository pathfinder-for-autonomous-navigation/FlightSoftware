#include "QuakeManager.h"
#include "Drivers/QLocate.hpp"

#include "radio_state_t.enum"

// Include I/O functions for telemetry dumping during functional testing.
#ifndef FLIGHT
#ifdef DESKTOP
#include <iostream>
#include <sstream>
#include <iomanip>
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
QuakeManager::QuakeManager(StateFieldRegistry &registry, unsigned int offset)
    : TimedControlTask<void>(registry, "quake", offset),
      max_wait_cycles_f("radio.max_wait", Serializer<unsigned int>(PAN::one_day_ccno)),
      max_transceive_cycles_f("radio.max_transceive", Serializer<unsigned int>(PAN::one_day_ccno)),
      radio_err_f("radio.err", Serializer<int>(-90, 10)),
      radio_mt_packet_f("uplink.ptr"),
      radio_mt_len_f("uplink.len"),
      radio_state_f("radio.state", Serializer<unsigned char>()),
      last_checkin_cycle_f("radio.last_comms_ccno", Serializer<unsigned int>()), // Last communication control cycle #
      dump_telemetry_f("telem.dump", Serializer<bool>()),
      qct(),
      mo_idx(0),
      unexpected_flag(false)
{
    add_writable_field(max_wait_cycles_f);
    add_writable_field(max_transceive_cycles_f);
    add_readable_field(radio_err_f);
    add_internal_field(radio_mt_packet_f);
    add_internal_field(radio_mt_len_f);
    add_readable_field(radio_state_f);
    add_readable_field(last_checkin_cycle_f);
    add_writable_field(dump_telemetry_f);

    // Retrieve fields from registry
    snapshot_size_fp = find_internal_field<size_t>("downlink.snap_size", __FILE__, __LINE__);
    radio_mo_packet_fp = find_internal_field<char *>("downlink.ptr", __FILE__, __LINE__);
    radio_power_cycle_fp = find_writable_field<bool>("gomspace.power_cycle_output3_cmd", __FILE__, __LINE__);

    cycle_of_entry = control_cycle_count;

    // Initialize Quake Manager variables
    max_wait_cycles_f.set(1);
    max_transceive_cycles_f.set(500);
    last_checkin_cycle_f.set(control_cycle_count);
    radio_mt_packet_f.set(qct.get_MT_msg());
    radio_mt_len_f.set(0);
    // Radio initializes to the disabled state
    radio_state_f.set(static_cast<unsigned int>(radio_state_t::disabled));
    dump_telemetry_f.set(false);
}

void QuakeManager::init(){
    // Setup MO Buffers
    max_snapshot_size = std::max(snapshot_size_fp->get() + 1, static_cast<size_t>(packet_size));
    mo_buffer_copy = new char[max_snapshot_size]();
}

QuakeManager::~QuakeManager()
{
    delete[] mo_buffer_copy;
}

#ifndef FLIGHT
void QuakeManager::dump_debug_telemetry(char *buffer, size_t size){
    #ifdef DESKTOP
    std::cout << "{\"t\":" << debug_console::_get_elapsed_time() << ",\"telem\":\"";
    for (size_t i = 0; i < size; i++)
    {
        std::ostringstream out;
        out << "\\\\x";
        out << std::hex << std::setfill('0') << std::setw(2) << (0xFF & buffer[i]);
        std::cout << out.str();
    }
    std::cout << "\"}\n";
    #else
    Serial.printf("{\"t\":%d,\"telem\":\"", debug_console::_get_elapsed_time());
    for (size_t i = 0; i < size; i++)
    {
        Serial.print("\\\\x");
        Serial.print((0xFF & buffer[i]), HEX);
    }
    Serial.print("\"}\n");
    #endif
}
#endif

void QuakeManager::execute()
{
#ifndef FLIGHT
    if (dump_telemetry_f.get())
    {
        dump_telemetry_f.set(false);
        char *snapshot = radio_mo_packet_fp->get();

        dump_debug_telemetry(snapshot, snapshot_size_fp->get());
    }
#endif

    const auto radio_state = static_cast<radio_state_t>(radio_state_f.get());
    switch (radio_state)
    {
    case radio_state_t::disabled:
        dispatch_disabled();
        break;
    case radio_state_t::config:
        dispatch_config();
        break;
    case radio_state_t::wait:
        dispatch_wait();
        break;
    case radio_state_t::transceive:
        dispatch_transceive();
        break;
    case radio_state_t::read:
        dispatch_read();
        break;
    case radio_state_t::write:
        dispatch_write();
        break;
    default:
        printf(debug_severity::error, "Radio state not defined: %d",
               radio_state_f.get());
        // Default behavior is to set the error flag and go to wait
        unexpected_flag = true;
        dispatch_wait();
    }
}

void QuakeManager::dispatch_disabled()
{
    // This fixes the bug where quake would report that config ran out of cycles at startup
    transition_radio_state(radio_state_t::disabled);
}

void QuakeManager::dispatch_config()
{

    if (no_more_cycles(max_config_cycles))
    {
        return handle_err(Devices::TIMEOUT);
    }

    int err_code = qct.execute(radio_state_t::config);

    if (is_actual_error(err_code))
    {
        return handle_err(err_code);
    }

    // If we have finished executing this command, then transition to write
    if (has_finished())
    {
        mo_idx = 0; // make sure to write a new snapshot
        return transition_radio_state(radio_state_t::write);
    }
}

bool QuakeManager::has_finished() const
{
    return qct.get_fn_num() == 0 && control_cycle_count - cycle_of_entry > 1;
}

void QuakeManager::dispatch_wait()
{
    bool powered = !radio_power_cycle_fp->get();

    // If we still have cycles or we are powercycling, then just return
    if (!no_more_cycles(max_wait_cycles_f.get()) || !powered)
    {
        return;
    }
    // If the unexpected_flag was set, then clear it and call config
    if (unexpected_flag)
    {
        unexpected_flag = false;
        transition_radio_state(radio_state_t::config);
    }
    else
    {
        mo_idx = 0; // make sure to always write a new snapshot
        transition_radio_state(radio_state_t::write);
    }
}

void QuakeManager::dispatch_write()
{

    if (no_more_cycles(max_write_cycles))
    {
        return handle_err(Devices::TIMEOUT);
    }

    // If we just entered write, copy the current message to our local buf
    if (has_just_entered())
    {
        // If mo_idx is 0 then copy a new snapshot into our local buffer
        if (mo_idx == 0)
        {
            copy_next_snapshot();
        }
        // Set MO pointer to the next block
        copy_next_packet();
    }

    int err_code = qct.execute(radio_state_t::write);
    if (is_actual_error(err_code))
    {
        return handle_err(err_code);
    }

    if (has_finished())
    {
        transition_radio_state(radio_state_t::transceive);
    }
}

bool QuakeManager::has_just_entered() const
{
    return qct.get_fn_num() == 0 && control_cycle_count - cycle_of_entry <= 1;
}

void QuakeManager::copy_next_packet()
{
    // load the current 70 bytes of the buffer
    qct.set_downlink_msg(mo_buffer_copy + (packet_size * mo_idx), packet_size);
    #if !defined(FLIGHT) && defined(AUTOTELEM)
    // printf(debug_severity::error, "Attempting to Dump Telemetry\n");
    dump_debug_telemetry(mo_buffer_copy + (packet_size * mo_idx), packet_size);
    #endif
    assert(max_snapshot_size / packet_size != 0);
    mo_idx = (mo_idx + 1) % (max_snapshot_size / packet_size);
}

void QuakeManager::copy_next_snapshot()
{
    memset(mo_buffer_copy, 0, sizeof(*mo_buffer_copy) * max_snapshot_size);
    memcpy(mo_buffer_copy, radio_mo_packet_fp->get(), max_snapshot_size);
}

void QuakeManager::dispatch_transceive()
{

    // If we run out of cycles --> then this is an error because any given SBDIX transaction should finish within
    // max_transceive_cycles
    if (no_more_cycles(max_transceive_cycles_f.get()))
    {
        return handle_err(Devices::TIMEOUT);
    }

    int err_code = qct.execute(radio_state_t::transceive);

    if (is_actual_error(err_code))
    {
        return handle_err(err_code);
    }

    // If we have finished executing SBDIX, then see if we have a message
    if (has_finished())
    {
        // Case 1: We have no comms --> try again until we run out of cycles
        if (qct.get_MO_status() > 4)
        {
            return handle_no_comms();
        }

        #if !(defined(DESKTOP)) || defined(UNIT_TEST)
            // We have comms, so update the last_checkin time
            last_checkin_cycle_f.set(control_cycle_count);
        #endif

        // Case 2: We have comms and we have message --> read message
        if (qct.get_MT_status() == 1) // SBD message successfully retrieved
        {
            handle_comms_msg();
        }
        // Case 3: We have comms and we have no message --> load next message
        else
        {
            handle_comms_no_msg();
        }
    }
}

void QuakeManager::handle_no_comms()
{
    // If we have at least 1/3 of max_transceive_cycles left, then try transceive again
    unsigned int cycles_left = max_transceive_cycles_f.get() - (control_cycle_count - cycle_of_entry);
    // TODO: arbitrary 1/3
    if (cycles_left > (max_transceive_cycles_f.get() / 3) + 1) // +1 so that it doesnt round to zero
    {
        return;
    }
    else
    {
        mo_idx = 0; // Make sure we get a new snapshot
        return transition_radio_state(radio_state_t::write);
    }
}

void QuakeManager::handle_comms_msg()
{
    printf(debug_severity::info,
           "[Quake Info] SBDIX finished, transitioning to SBDRB");
    transition_radio_state(radio_state_t::read);
}

void QuakeManager::handle_comms_no_msg()
{
    // printf(debug_severity::info,
    //     "[Quake Info] SBDIX finished, transitioning to SBDWB");
    transition_radio_state(radio_state_t::write);
}

void QuakeManager::dispatch_read()
{

    if (no_more_cycles(max_read_cycles))
    {
        return handle_err(Devices::TIMEOUT);
    }

    int err_code = qct.execute(radio_state_t::read);

    if (is_actual_error(err_code))
    {
        return handle_err(err_code);
    }

    // If we are done with SBDRB --> save message and load next message
    if (has_finished())
    {
        printf(debug_severity::info,
               "[Quake Info] SBDRB finished, transitioning to SBDWB");

        radio_mt_len_f.set(qct.get_MT_length());

        transition_radio_state(radio_state_t::write);
    }
}

bool QuakeManager::is_actual_error(int err_code)
{
    // This logic is easy to mistype, so make it a function
    return err_code != Devices::OK && err_code != Devices::PORT_UNAVAILABLE;
}

void QuakeManager::handle_err(int err_code)
{
    radio_err_f.set(err_code);
    unexpected_flag = true;
    transition_radio_state(radio_state_t::wait);

    printf(debug_severity::error,
           "[Quake Error] Execution failed at radio state %d and fn_number %d with error code %d",
           radio_state_f.get(),
           qct.get_fn_num(),
           err_code);
}

bool QuakeManager::no_more_cycles(size_t max_cycles)
{
    return control_cycle_count - cycle_of_entry > max_cycles;
}

void QuakeManager::transition_radio_state(radio_state_t new_state)
{
    // Reset the num of cycles here, reset fnSeqNum
    cycle_of_entry = control_cycle_count;
    qct.fnSeqNum = 0;
    radio_state_f.set(static_cast<unsigned int>(new_state));
}
