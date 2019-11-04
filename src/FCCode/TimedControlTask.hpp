#ifndef TIMED_CONTROL_TASK_HPP_
#define TIMED_CONTROL_TASK_HPP_

#include <ControlTask.hpp>

#ifdef DESKTOP
#include <thread>
#include <chrono>

template<typename T>
class TimedControlTask : public ControlTask<T> {
  using namespace std::chrono;
  typedef time_point<system_clock, milliseconds> timepoint;

  private:
    /**
     * @brief The start time of this control task, relative
     * to the start of any control cycle, in microseconds.
     */
    timepoint::duration offset;
  public:
    TimedControlTask(
                     StateFieldRegistry& registry,
                     const timepoint::duration& _offset) :
      ControlTask<T>(registry), offset(_offset)
    {} 

    T execute_timed(const timepoint& control_cycle_start_time) {
      timepoint earliest_start_time = control_cycle_start_time + offset;
      std::this_thread::sleep_until(earliest_start_time);

      execute();
    }
};

#else
#include <Arduino.h>

template<typename T>
class TimedControlTask : public ControlTask<T> {
  private:
    /**
     * @brief The start time of this control task, relative
     * to the start of any control cycle, in microseconds.
     */
    unsigned int offset;
  public:
    TimedControlTask(
                     StateFieldRegistry& registry,
                     unsigned int _offset) :
      ControlTask<T>(registry), offset(_offset)
    {} 

    T execute_timed(unsigned int control_cycle_start_time) {
      unsigned int earliest_start_time = control_cycle_start_time + offset;
      while ((signed int)(earliest_start_time - micros()) > 0) {
        delayMicroseconds(100);
      }

      execute();
    }
};

#endif

#endif