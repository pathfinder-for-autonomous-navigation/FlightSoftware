#include "DebugTask.hpp"

#ifdef HOOTL
DebugTask::DebugTask(StateFieldRegistry &registry, unsigned int offset)
    : TimedControlTask<void>(registry, offset),
      start_cycle_f("cycle.start", Serializer<bool>()) {
  add_writable_field(start_cycle_f);
  init();
}
#else
DebugTask::DebugTask(StateFieldRegistry &registry, unsigned int offset)
    : TimedControlTask<void>(registry, offset) {
  init();
}
#endif

void DebugTask::execute() {
  #ifdef HOOTL
    start_cycle_f.set(false);
    while (!start_cycle_f.get())
      process_commands(_registry);
  #endif
}

void DebugTask::init() { debug_console::init(); }
