#ifndef DUMMY_CONTROL_TASK_HPP_
#define DUMMY_CONTROL_TASK_HPP_

#include <ControlTask.hpp>

class DummyControlTask : public ControlTask<void> {
  public:
    DummyControlTask(StateFieldRegistry& registry) : ControlTask<void>(registry) {}
    void execute() { 
      DummyControlTask::x = 3;
    }
    int x = 2;
};

#endif
