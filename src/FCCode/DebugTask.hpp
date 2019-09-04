#ifndef DEBUG_TASK_HPP_
#define DEBUG_TASK_HPP_

#include "ControlTask.hpp"

class DebugTask : public ControlTask {
    protected:
        const debug_console dbg;
    
};

#endif