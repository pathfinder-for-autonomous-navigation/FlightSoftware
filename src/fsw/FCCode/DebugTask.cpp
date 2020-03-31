#include "DebugTask.hpp"

#ifdef FUNCTIONAL_TEST
DebugTask::DebugTask(StateFieldRegistry &registry, unsigned int offset)
    : TimedControlTask<void>(registry, "debug", offset),
      start_cycle_f("cycle.start", Serializer<bool>(),
      auto_cycle_f("cycle.auto", Serializer<bool>())) {
  add_writable_field(start_cycle_f);
  add_writable_field(auto_cycle_f);
  init();
}
#else
DebugTask::DebugTask(StateFieldRegistry &registry, unsigned int offset)
    : TimedControlTask<void>(registry, "debug", offset) {
  init();
}
#endif

void DebugTask::execute() {
#ifdef FUNCTIONAL_TEST
  start_cycle_f.set(false);

  if(auto_cycle_f.get()){
  while (!start_cycle_f.get())
    process_commands(_registry);
  }
  else{
    process_commands(_registry);
  }
#endif
}

void DebugTask::init() {
 #ifndef GSW
    debug_console::init();
 #endif
}
