#include "ClockManager.hpp"

ClockManager::ClockManager(StateFieldRegistry &registry,
                           const unsigned int _control_cycle_size) :
    TimedControlTask<void>(registry, 0),
    control_cycle_size(_control_cycle_size),
    control_cycle_count_f("pan.cycle_no", Serializer<unsigned int>())
{
    add_readable_field(control_cycle_count_f);
}

void ClockManager::execute() {
    if (has_executed) {
        sys_time_t earliest_start_time =
            TimedControlTask<void>::control_cycle_start_time + control_cycle_size;
        wait_until_time(earliest_start_time);
    }

    has_executed = true;
    TimedControlTask<void>::control_cycle_start_time = get_system_time();
    control_cycle_count_f.set(control_cycle_count_f.get() + 1);
}
