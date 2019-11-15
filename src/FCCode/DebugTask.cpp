#include "DebugTask.hpp"

DebugTask::DebugTask(StateFieldRegistry& registry) : ControlTask(registry) { init(); }

void DebugTask::execute() { process_commands(_registry); }

void DebugTask::init() { debug_console::init(); }
