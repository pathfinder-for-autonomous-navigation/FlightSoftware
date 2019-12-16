#include "ClockManager.hpp"

ClockManager::ClockManager(StateFieldRegistry &registry,
                           const unsigned int _control_cycle_size) :
    TimedControlTask<void>(registry, "clock_ct", 0),
    control_cycle_size(_control_cycle_size),
    control_cycle_count_f("pan.cycle_no", Serializer<unsigned int>())
{
    add_readable_field(control_cycle_count_f);
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
