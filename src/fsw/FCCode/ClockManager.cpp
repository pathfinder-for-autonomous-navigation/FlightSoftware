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
    Fault::cc = &TimedControlTaskBase::control_cycle_count;
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

unsigned int ClockManager::systime_to_cycle(sys_time_t time) {
    systime_duration_t duration = time - initial_start_cycling_time;
    return duration/control_cycle_size;
}

sys_time_t ClockManager::cycle_to_systime(unsigned int ccno) {
    systime_duration_t duration = ccno * control_cycle_size;
    return initial_start_cycling_time+duration;
}