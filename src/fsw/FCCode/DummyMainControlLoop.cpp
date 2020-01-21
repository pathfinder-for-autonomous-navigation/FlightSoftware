#include "DummyMainControlLoop.hpp"
#include "DebugTask.hpp"

DummyMainControlLoop::DummyMainControlLoop(StateFieldRegistry& registry)
    : ControlTask<void>(registry), debug_task(registry, 0), 
    value_sr(40, 125, 10),
    readable_f("readable_field", value_sr),
    writable_f("writable_field", value_sr)
{
    add_readable_field(readable_f);
    add_writable_field(writable_f);
}

void DummyMainControlLoop::execute() {
    debug_task.execute();
}

void DummyMainControlLoop::init() {
    debug_task.init();
}
