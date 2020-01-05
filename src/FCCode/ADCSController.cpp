#include "ADCSController.hpp"

#include <adcs_constants.hpp>

ADCSController::ADCSController(StateFieldRegistry &registry, 
    unsigned int offset, Devices::ADCS &_adcs)
    : TimedControlTask<void>(registry, "adcs_monitor", offset),
    adcs_system(_adcs)
    {
        
    }

void ADCSController::execute(){

}