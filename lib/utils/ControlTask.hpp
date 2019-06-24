#ifndef CONTROL_TASK_HPP_
#define CONTROL_TASK_HPP_

#include <string>
#include <Debuggable.hpp>
#include <Nameable.hpp>

/**
 * @brief Dummy class used so that we can produce generic pointers to
 * ControlTasks.
 */
class Task : public Nameable {
  public:
    using Nameable::Nameable;
};

/**
 * @brief A control task is a task that is executed whilst within a state machine.
 * 
 * This class is subclassed heavily in order to provide multiple implementations 
 * of execute() for different kinds of ControlTasks.
 */
template<typename T>
class ControlTask : public Task, Debuggable {
  public:
    /**
     * @brief Construct a new Control Task object
     */
    ControlTask(const std::string& name, debug_console& dbg);
    /**
     * @brief Run main method of control task.
     */
    virtual T execute() = 0;
};

template <typename T>
ControlTask<T>::ControlTask(const std::string& name, 
                            debug_console& dbg) : 
    Task(name),
    Debuggable(dbg) {}

/**
 * @brief Creates a wrapper around the ChibiOS thread interface
 * for a ControlTask. Allows for creation and execution of "deferred"
 * tasks that execute some time after their initialization.
 */
template<size_t thd_working_area>
class ThreadedTask : public ControlTask<void> {
  protected:
    virtual_timer_t timer;
    stkalign_t working_area[thd_working_area];
    thread_t* thd;

    bool is_timer_initialized;
    bool is_thd_created;
  public:
    ThreadedTask(const std::string& name, debug_console& debug);

    void start_now(void* args = nullptr);
    void start(const systime_t delay = static_cast<systime_t>(0), void* args = nullptr);
    
    virtual void execute() = 0;
};

template<size_t thd_working_area>
ThreadedTask<thd_working_area>::ThreadedTask(const std::string& name, debug_console& debug) : 
                                ControlTask<void>(name, debug), 
                                timer(),
                                working_area(),
                                thd(nullptr),
                                is_timer_initialized(false),
                                is_thd_created(false) { }

template<size_t thd_working_area>
void ThreadedTask<thd_working_area>::start_now(void* args) {
  if (!is_thd_created) {
    // Create thread prior to execution if not created
    thread_descriptor_t descriptor = {
      name.c_str(),
      this->working_area,
      this->working_area + thd_working_area,
      NORMALPRIO,
      this->execute,
      args,
    };
    this->thd = chThdCreateI(&descriptor);
  }

  chThdStartI(this->thd);
}

template<size_t thd_working_area>
void ThreadedTask<thd_working_area>::start(const systime_t delay, void* args) {
  if (static_cast<unsigned int>(delay) == 0) {
    start_now();
    return;
  }

  if (!is_timer_initialized) {
    chVTObjectInit(&(this->timer));
  }

  // Start timer to execute the task, resetting it if necessary
  chVTSetI(&(this->timer), delay, this->start_now, args);
}

class StateHandler : public ControlTask<unsigned int> {
  public:
    StateHandler(const std::string& name, debug_console& dbg, bool only_once = false);
    virtual unsigned int execute() = 0;

    bool only_execute_once;
    bool has_executed;
};

class TransitionHandler : public ControlTask<void> {
  public:
    using ControlTask<void>::ControlTask;
    virtual void execute() = 0;
};

#endif