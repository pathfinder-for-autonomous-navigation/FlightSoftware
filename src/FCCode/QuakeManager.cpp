#include "QuakeManager.h"

QuakeManager::QuakeManager(StateFieldRegistry &registry) : ControlTask<void>(registry),
    radio_mode_sr(0, 10, 4),
    radio_mode_f("radio.mode", radio_mode_sr),
#ifdef DESKTOP
    qct()
#else 

#endif
{ 
    control_cycle_count_fp = find_readable_field<unsigned int>("pan.cycle_no", __FILE__, __LINE__);
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
    // Construct the Quake Control Task object

}

void QuakeManager::dispatch_manual(){

}
