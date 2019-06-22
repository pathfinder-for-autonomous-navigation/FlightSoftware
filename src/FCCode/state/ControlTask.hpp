#ifndef CONTROL_TASK_HPP_
#define CONTROL_TASK_HPP_

#include <string>
#include <Debuggable.hpp>
#include <Nameable.hpp>

/**
 * @brief Dummy class used so that we can produce generic pointers to
 * ControlTasks.
 */
class Task {};

/**
 * @brief A control task is a task that is executed whilst within a state machine.
 * 
 * This class is subclassed heavily in order to provide multiple implementations 
 * of execute() for different kinds of ControlTasks.
 */
template<typename T>
class ControlTask : public Task, Nameable, Debuggable {
  public:
    /**
     * @brief Construct a new Control Task object
     */
    ControlTask(std::string& name, debug_console& dbg);
    /**
     * @brief Run main method of control task.
     */
    virtual T execute() = 0;
};

template <typename T>
ControlTask<T>::ControlTask(std::string& name, 
                            debug_console& dbg) : 
    Task(),
    Nameable(name),
    Debuggable(dbg) {}

#endif