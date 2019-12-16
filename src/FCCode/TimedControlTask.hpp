#ifndef TIMED_CONTROL_TASK_HPP_
#define TIMED_CONTROL_TASK_HPP_

#include "ControlTask.hpp"
#include "constants.hpp"
#include <string>

#ifdef DESKTOP
#include <thread>
#include <chrono>
#include <time.h>
#else
#include <Arduino.h>
#endif

/**
 * The sys_time_t and systime_duration_t are common types for tracking system
 * time across both the desktop and the native platforms.
 * 
 * - sys_time_t denotes an absolute time
 * - systime_duration_t denotes a unitless separation between two system times
 * 
 * On Teensies, these quantities are all unsigned ints representing microseconds. On
 * desktop platforms, the std::chrono library is used to provide meaning to each of
 * these two constructs. See below.
 */
#ifdef DESKTOP
typedef std::chrono::steady_clock::time_point sys_time_t;
typedef std::chrono::steady_clock::duration systime_duration_t;
#else
typedef unsigned int sys_time_t;
typedef unsigned int systime_duration_t;
#endif

/**
 * @brief Timing values and functions that are shared across all timed control tasks,
 * irrespective of return type.
 */
class TimedControlTaskBase {
  protected:
    /**
     * @brief The time at which the current control cycle started.
     */
    static sys_time_t control_cycle_start_time;
    

  public:
    static unsigned int control_cycle_count;

    /**
     * @brief Get the system time.
     * 
     * @return sys_time_t
     */
    static sys_time_t get_system_time() {
      #ifdef DESKTOP
        return std::chrono::steady_clock::now();
      #else
        return micros();
      #endif
    }

    /**
     * @brief Convert a duration object into microseconds.
     * 
     * @param delta 
     * @return systime_duration_t 
     */
    static unsigned int duration_to_us(const systime_duration_t& delta) {
      #ifdef DESKTOP
        return std::chrono::duration_cast<std::chrono::microseconds>(delta).count();
      #else
        return delta;
      #endif
    }

    /**
     * @brief Convert microseconds into a duration object.
     * 
     * @param delta 
     * @return systime_duration_t 
     */
    static systime_duration_t us_to_duration(const unsigned int delta) {
      #ifdef DESKTOP
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::microseconds(delta));
      #else
        return delta;
      #endif
    }

    static void wait_duration(const unsigned int& delta_t) {
      const sys_time_t start = get_system_time();
      // Wait until execution time
      while(duration_to_us(get_system_time() - start) < delta_t) {
        #ifndef DESKTOP
          delayMicroseconds(10);
        #endif
      }
    }
};

/**
 * @brief Task-specific methods and values for timed control tasks.
 * 
 * @tparam T Return type of control task.
 */
template<typename T>
class TimedControlTask : public ControlTask<T>, public TimedControlTaskBase {
  private:
    /**
     * @brief The start time of this control task, relative
     * to the start of any control cycle, in microseconds.
     */
    systime_duration_t offset;

    /**
     * @brief Number of times a timed control task has not had
     * any time to wait.
     */
    std::string num_lates_field_name;
    ReadableStateField<unsigned int> num_lates_f;

    /**
     * @brief Number of times a control task
     */
    std::string avg_wait_field_name;
    ReadableStateField<float> avg_wait_f;

  public:
    /**
     * @brief Execute this control task's task, but only if it's reached its
     * start time.
     * 
     * @param control_cycle_start_time System time for the start of the control task.
     * @return T Value returned by execute().
     */
    T execute_on_time() {
      sys_time_t earliest_start_time = 
        TimedControlTaskBase::control_cycle_start_time + offset;
      wait_until_time(earliest_start_time);
      return this->execute();
    }

    /**
     * @brief Cause the system to pause operation until a system time is reached.
     * 
     * TODO check for errors that could happen on Teensy due to integer overflow.
     * 
     * @param time Time until which the system should pause.
     */
    void wait_until_time(const sys_time_t& time) {
      // Compute timing statistics and publish them to state fields
      const signed int delta_t = (signed int) duration_to_us(time - get_system_time());
      if (delta_t <= 0) {
        num_lates_f.set(num_lates_f.get() + 1);
      }
      const unsigned int wait_time = std::max(delta_t, 0);
      const float new_avg_wait = ((avg_wait_f.get() * control_cycle_count) + wait_time) /
        (control_cycle_count + 1);
      avg_wait_f.set(new_avg_wait);

      wait_duration(wait_time); 
    }

    /**
     * @brief Construct a new Timed Control Task object
     * 
     * @param registry State field registry
     * @param name Name of control task (used in producing state fields for timing statistics)
     * @param offset Time offset of start of this task from the beginning of a
     *               control cycle, in microseconds.
     */
    TimedControlTask(StateFieldRegistry& registry,
                     const std::string& name,
                     const unsigned int _offset) :
        ControlTask<T>(registry),
        offset(us_to_duration(_offset + 1)),
        num_lates_field_name("timing." + name + ".num_lates"),
        num_lates_f(num_lates_field_name, Serializer<unsigned int>()),
        avg_wait_field_name("timing." + name + ".avg_wait"),
        avg_wait_f(avg_wait_field_name, Serializer<float>(0,PAN::control_cycle_time_us,32))
    {
      this->add_readable_field(num_lates_f);
      this->add_readable_field(avg_wait_f);
    }
};

#endif
