#include "DebugTask.hpp"

DebugTask::DebugTask(StateFieldRegistry &registry)
    : TimedControlTask<void>(registry, "debug"),
      start_cycle_f("cycle.start", Serializer<bool>(), 1 ),
      auto_cycle_f("cycle.auto", Serializer<bool>(), 1 ) {
  add_writable_field(start_cycle_f);
  add_writable_field(auto_cycle_f);
  auto_cycle_f.set(false);
  init();
}

DebugTask::~DebugTask() {
#ifndef GSW
    debug_console::close();
#endif
}

void DebugTask::execute() {
#ifndef FLIGHT
  start_cycle_f.set(false);
  if(auto_cycle_f.get()){
    process_commands(_registry);
  }
  else{
    // !auto_cycle_f.get() is false once auto_cycle_f is true, ending the while loop
    while (!start_cycle_f.get() && !auto_cycle_f.get()) 
      process_commands(_registry);
  }

#endif
}

void DebugTask::init() {
#ifndef GSW
    debug_console::open();
#endif
}
