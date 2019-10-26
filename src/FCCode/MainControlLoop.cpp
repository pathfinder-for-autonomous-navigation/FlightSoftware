#include "MainControlLoop.hpp"
#include "DebugTask.hpp"

MainControlLoop::MainControlLoop(StateFieldRegistry& registry)
    : ControlTask<void>(registry), debug_task(registry), 
    value_sr(-40, 125, Serializer<signed int>::temp_sz),
    readable_f("readable_field", value_sr),
    writable_f("writable_field", value_sr)
{
    add_readable_field(readable_f);
    add_writable_field(writable_f);
}

void MainControlLoop::execute() {
    debug_task.execute();
}

void MainControlLoop::init() {
    debug_task.init();
}
