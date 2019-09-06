#ifndef DEBUG_TASK_HPP_
#define DEBUG_TASK_HPP_

#include "ControlTask.hpp"

class DebugTask : public ControlTask<void> {
   public:
    DebugTask(StateFieldRegistry& registry) : ControlTask("debug_task", registry) { init(); }
    void execute() override { process_commands(_registry); }
};

#endif