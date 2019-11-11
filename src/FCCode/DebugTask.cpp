#include "DebugTask.hpp"

DebugTask::DebugTask(StateFieldRegistry& registry, unsigned int offset) :
    TimedControlTask<void>(registry, offset) { init(); }

void DebugTask::execute() { process_commands(_registry); }

void DebugTask::init() { debug_console::init(); }
