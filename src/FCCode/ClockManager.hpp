#ifndef MISSION_MANAGER_HPP_
#define MISSION_MANAGER_HPP_

#include "TimedControlTask.hpp"

class ClockManager : public TimedControlTask<void> {
   public:
    ClockManager(StateFieldRegistry& registry, const unsigned int control_cycle_size);
    void execute() override;

    systime_t get_control_cycle_start_time();
   private:
    /**
     * @brief If no control cycle has ended yet, this is set to false.
     * Otherwise true.
     */
    bool has_executed = false;

    /**
     * @brief Start time of the most recent control cycle that ended.
     * If no control cycle has ended yet, then this value is undefined.
     */
    systime_t control_cycle_start_time;

    /**
     * @brief Duration of a control cycle in microseconds.
     */
    systime_duration_t control_cycle_size;

    /**
     * @brief Keeps track of the current control cycle count.
     */
    Serializer<unsigned int> control_cycle_count_sr;
    ReadableStateField<unsigned int> control_cycle_count_f;
};

#endif
