#ifndef TIMED_CONTROL_TASK_HPP_
#define TIMED_CONTROL_TASK_HPP_

#include <ControlTask.hpp>

#ifdef DESKTOP
#include <thread>
#include <chrono>
#else
#include <Arduino.h>
#endif

/**
 * The systime_t and systime_duration_t are common types for tracking system
 * time across both the desktop and the native platforms.
 * 
 * - systime_t denotes an absolute time
 * - systime_duration_t denotes a unitless separation between two system times
 * 
 * On Teensies, these quantities are all unsigned ints representing microseconds. On
 * desktop platforms, the std::chrono library is used to provide meaning to each of
 * these two constructs. See below.
 */
#ifdef DESKTOP
typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>
    systime_t;
typedef systime_t::duration systime_duration_t;
#else
typedef unsigned int systime_t;
typedef unsigned int systime_duration_t;
#endif

template<typename T>
class TimedControlTask : public ControlTask<T> {
  public:
    /**
     * @brief Execute this control task's task, but only if it's reached its
     * start time.
     * 
     * @param control_cycle_start_time System time for the start of the control task.
     * @return T Value returned by execute().
     */
    T execute_on_time(const systime_t& control_cycle_start_time) {
      systime_t earliest_start_time = control_cycle_start_time + offset;
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
    void wait_until_time(const systime_t& time) const {
      #ifdef DESKTOP
        std::this_thread::sleep_until(time);
      #else
        while ((signed int)(time - micros()) > 0) {
          delayMicroseconds(10);
        }
      #endif
    }

    /**
     * @brief Get the system time.
     * 
     * @return systime_t
     */
    systime_t get_system_time() const {
      #ifdef DESKTOP
        return std::chrono::system_clock::now();
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
    const unsigned int duration_to_us(const systime_duration_t& delta) const {
      #ifdef DESKTOP
        return delta.count();
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
    systime_duration_t us_to_duration(const unsigned int delta) const {
      #ifdef DESKTOP
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::microseconds(delta));
      #else
        return delta;
      #endif
    }

    /**
     * @brief Return the duration between two system times as an object.
     * 
     * @param t1 The earlier time.
     * @param t2 The later time.
     * @return Duration.
     */
    systime_duration_t get_time_delta(const systime_t& t1, const systime_t& t2) const {
      return t2 - t1;
    }

    /**
     * @brief Construct a new Timed Control Task object
     * 
     * @param registry State field registry
     * @param offset Time offset of start of this task from the beginning of a
     *               control cycle, in microseconds.
     */
    TimedControlTask(
                      StateFieldRegistry& registry,
                      const unsigned int _offset) :
        ControlTask<T>(registry), offset(us_to_duration(_offset))
    {}
  private:
    /**
     * @brief The start time of this control task, relative
     * to the start of any control cycle, in microseconds.
     */
    systime_duration_t offset;
};

#endif
