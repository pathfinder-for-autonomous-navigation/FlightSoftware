#include "DebugTask.hpp"

DebugTask::DebugTask(StateFieldRegistry& registry) : ControlTask("debug_task", registry) { init(); }

void DebugTask::execute() { process_commands(_registry); }