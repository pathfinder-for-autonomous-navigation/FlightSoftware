#include "ClockManager.hpp"
#include <common/Event.hpp>

ClockManager::ClockManager(StateFieldRegistry &registry,
                           const unsigned int _control_cycle_size) :
    TimedControlTask<void>(registry, "clock_ct", 0),
    control_cycle_size(_control_cycle_size),
    control_cycle_count_f("pan.cycle_no", Serializer<unsigned int>())
{
    add_readable_field(control_cycle_count_f);
    Event::ccno = &control_cycle_count_f;
    initial_start_cycling_time = get_system_time();
}

void ClockManager::execute() {
    if (has_executed) {
        sys_time_t earliest_start_time =
            TimedControlTaskBase::control_cycle_start_time + control_cycle_size;
        wait_until_time(earliest_start_time);
    }

    has_executed = true;
    TimedControlTaskBase::control_cycle_start_time = get_system_time();
    control_cycle_count++;
    control_cycle_count_f.set(control_cycle_count);
}

static sys_time_t ClockManager::cycle_to_systime(const unsigned int ccno) {
    // Get the amount of time that has passed since starting the clock in microseconds
    #ifdef DESKTOP
    systime_duration_t duration = 
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::microseconds(ccno*control_cycle_size));
    #else
    systime_duration_t duration = ccno*control_cycle_size;
    #endif
    
    return initial_start_cycling_time+duration;
}

/**
 * @brief Convert a system time to the control cycle number
 */
static unsigned int ClockManager::systime_to_cycle(const sys_time_t time) {
    systime_duration_t duration = us_to_duration(time - initial_start_cycling_time);
    return duration/control_cycle_size;
}