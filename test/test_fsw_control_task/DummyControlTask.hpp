#ifndef DUMMY_CONTROL_TASK_HPP_
#define DUMMY_CONTROL_TASK_HPP_

#include <iostream>
#include <ControlTask.hpp>

class DummyControlTask : public ControlTask<void> {
  public:
    DummyControlTask(StateFieldRegistry& registry) : ControlTask<void>(registry) {}
    void execute() { std::cout << "I'm a dummy task" << std::endl; }
    const int x = 2;
};

#endif
