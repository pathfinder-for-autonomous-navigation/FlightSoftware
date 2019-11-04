
#include "QuakeManager.h"
#include "QLocate.hpp"

// Quake driver setup is initialized when QuakeController constructor is called
QuakeManager::QuakeManager(StateFieldRegistry &registry) : ControlTask<void>(registry),
    radio_mode_sr(0, 10, 4),
    radio_mode_f("radio.mode", radio_mode_sr),
    radio_msg_queue_fp("radio.mo_msg_queue"),
    qct(registry)
{ 
    control_cycle_count_fp = find_readable_field<unsigned int>("pan.cycle_no", __FILE__, __LINE__);
    add_writable_field(radio_mode_f);

    radio_mode_f.set(static_cast<unsigned int>(radio_mode_t::manual));
}

void QuakeManager::execute() {
    radio_mode_t mode = static_cast<radio_mode_t>(radio_mode_f.get());
    switch(mode){
        case radio_mode_t::startup:
        dispatch_startup();
        break;
        case radio_mode_t::manual:
        dispatch_manual();
        break;
        case radio_mode_t::waiting:
        dispatch_waiting();
        break;
        case radio_mode_t::transceiving:
        dispatch_transceiving();
        break;
        default:
            printf(debug_severity::error, "Radio state not defined: %d\n", static_cast<unsigned int>(mode));
            break;

    }
}

void QuakeManager::dispatch_startup() {
    int err_code = -1;
    int fn_number = qct.get_current_fn_number();
    // Determine what stage of config we are in
    switch (fn_number){
        case 0: 
            if (qct.request_state(CONFIG)) // this should always work
                radio_mode_f.set(static_cast<unsigned int>(radio_mode_t::startup));
            break;  
        case 1: 
        case 2:
            err_code = qct.execute();
            break;
        case 3:
            err_code = qct.execute();
            // When finished with startup, go into manual mode
            radio_mode_f.set(static_cast<unsigned int>(radio_mode_t::manual));
            break; 
        default:
            printf(debug_severity::error, "Radio startup fn_number not defined: %d\n", fn_number);
            return;

   }
    if(err_code != Devices::OK)
        printf(debug_severity::error, "Execution of Quake Config failed with code: %d at fn_number: %d\n", (err_code), fn_number);

}

void QuakeManager::dispatch_manual(){
    // In this mode keep checking for comms

}

void QuakeManager::dispatch_waiting(){
    // See how long we have been waiting for
    unsigned int delta = control_cycle_count_fp->get() - last_checkin_cycle;
    // If we have waited for longer than 5 minutes, check for comms
    if (delta > 3000) 
    {
        int state = qct.get_current_state();
        int err_code = -1;
        if (state == IDLE){
            if (qct.request_state(SBDIX)){
                radio_mode_f.set(static_cast<unsigned int>(radio_mode_t::manual));
                // Request SBDIX
                err_code = qct.execute();
            }else{
                printf(debug_severity::error, "Unable to transition Quake state to SBDIX from %s\n", translate_state(qct.get_current_state()));
                return;
            }
  
        }else if (state == SBDIX){
            // Get SBDIX response
            err_code = qct.execute();
            // State should be IDLE after this execute as long as  it was successful
            if (err_code == Devices::OK){
                const int* sbdix_response = qct.quake.sbdix_r;
                
                if (sbdix_response[0] < 5){ // Downlink was successful
                    // Update last checkin_cycle
                    last_checkin_cycle = control_cycle_count_fp->get();
                    // go into transceiving mode
                    radio_mode_f.set(static_cast<unsigned int>(radio_mode_t::transceiving));
                }
        }
        if (err_code != Devices::OK || err_code != Devices::PORT_UNAVAILABLE) // don't log waiting
            printf(debug_severity::error, "SBDIX execution failed to execute at fn_number: %d with error code %d\n", qct.get_current_fn_number(), (err_code));
    }
}


