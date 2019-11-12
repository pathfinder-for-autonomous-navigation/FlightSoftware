#include "MainControlLoop.hpp"
#include "DebugTask.hpp"

#ifdef HOOTL
#define DEBUG_TASK debug_task(registry, debug_task_offset),
#elif FLIGHT
#define DEBUG_TASK
#endif

MainControlLoop::MainControlLoop(StateFieldRegistry& registry)
    : ControlTask<void>(registry), 
      field_creator_task(registry),
      clock_manager(registry, 120000000),
      DEBUG_TASK
      mission_manager(registry, mission_manager_offset),
      docksys(),
      docking_controller(registry, docking_controller_offset, docksys)
{}

#undef DEBUG_TASK

void MainControlLoop::execute() {
    clock_manager.execute();
    sys_time_t control_cycle_start = clock_manager.get_control_cycle_start_time();

    #ifdef HOOTL
    debug_task.execute_on_time(control_cycle_start);
    #endif

    mission_manager.execute_on_time(control_cycle_start);
    docking_controller.execute_on_time(control_cycle_start);
}
