#include "ClockManager.hpp"

ClockManager::ClockManager(StateFieldRegistry &registry) : ControlTask<void>(registry), 
    control_cycle_count_sr(0, 4294967295, 32),
    control_cycle_count_f("pan.cycle_no", control_cycle_count_sr)
{
    add_readable(control_cycle_count_f);
}

void ClockManager::execute() {
    control_cycle_count_f.set(control_cycle_count_f.get() + 1);
}
