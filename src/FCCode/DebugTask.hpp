#ifndef DEBUG_TASK_HPP_
#define DEBUG_TASK_HPP_

#include "ControlTask.hpp"

class DebugTask : public ControlTask<void> {
   public:
    DebugTask(StateFieldRegistry& registry);
    void execute() override;
};

#endif
