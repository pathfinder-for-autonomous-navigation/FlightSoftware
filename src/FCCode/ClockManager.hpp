#ifndef CLOCK_MANAGER_HPP_
#define CLOCK_MANAGER_HPP_

#include "TimedControlTask.hpp"

class ClockManager : public TimedControlTask<void> {
   public:
    /**
     * @brief Construct a new Clock Manager
     * 
     * @param registry State field registry
     * @param control_cycle_size Control cycle time, in nanoseconds if on desktop,
     * and in microseconds if on Teensy.
     */
    ClockManager(StateFieldRegistry& registry, const unsigned int control_cycle_size);

    /**
     * @brief Increments the control cycle count and sets the control cycle
     * start time.
     * 
     * Also ensures that the control cycle count can only be incremented 
     * at least control_cycle_size microseconds after the last execution
     * of this function. Since this task is the first to run in any control
     * cycle, it therefore ensures that the control cycle stays within its
     * bounded values.
     */
    void execute() override;

   private:
    /**
     * @brief If no control cycle has ended yet, this is set to false.
     * Otherwise true.
     */
    bool has_executed = false;

    /**
     * @brief Duration of a control cycle.
     */
    systime_duration_t control_cycle_size;

    /**
     * @brief Keeps track of the current control cycle count.
     */
    ReadableStateField<unsigned int> control_cycle_count_f;
};

#endif
