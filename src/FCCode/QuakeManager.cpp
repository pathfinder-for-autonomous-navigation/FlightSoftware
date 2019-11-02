#include "QuakeManager.h"

QuakeManager::QuakeManager(StateFieldRegistry &registry) : ControlTask<void>(registry)
{ 
    control_cycle_count_fp = find_readable_field<unsigned int>("pan.cycle_no", __FILE__, __LINE__);
}

void QuakeManager::dispatch_startup() {

}

void QuakeManager::execute() {

}
