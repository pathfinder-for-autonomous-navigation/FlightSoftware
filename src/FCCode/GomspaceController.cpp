#include "GomspaceController.hpp"

GomspaceController::GomspaceController(StateFieldRegistry &registry, unsigned int offset,
    Devices::Gomspace &_gs)
    : TimedControlTask<void>(registry, offset), gs(_gs)
{

}

void GomspaceController::execute() {

}
