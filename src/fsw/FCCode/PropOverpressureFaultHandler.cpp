#include "PropOverpressureFaultHandler.hpp"

PropOverpressureFaultHandler::PropOverpressureFaultHandler(StateFieldRegistry& r) :
    SimpleFaultHandler(
        r,
        r.find_fault("prop.overpressured_base"),
        SimpleFaultHandler::active_state_lists[1],
        mission_state_t::standby
    )
{}

fault_response_t PropOverpressureFaultHandler::execute() {
    // TODO cause venting response
    return determine_recommended_state();
}
