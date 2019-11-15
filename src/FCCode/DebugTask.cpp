#include "DebugTask.hpp"

DebugTask::DebugTask(StateFieldRegistry& registry, unsigned int offset) :
    TimedControlTask<void>(registry, offset) { init(); }

void DebugTask::execute() {
    #ifdef HOOTL
    process_commands(_registry);
    #endif
}

void DebugTask::init() { debug_console::init(); }
