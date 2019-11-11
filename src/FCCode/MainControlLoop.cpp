#include "MainControlLoop.hpp"
#include "DebugTask.hpp"

MainControlLoop::MainControlLoop(StateFieldRegistry& registry)
    : ControlTask<void>(registry), 
      clock_manager(registry, 120000000),
      debug_task(registry, debug_task_offset),
      mission_manager(registry, mission_manager_offset),

      docksys(),
      docking_controller(registry, docking_controller_offset, docksys)
{}

void MainControlLoop::execute() {
    clock_manager.execute();
    sys_time_t control_cycle_start = clock_manager.get_control_cycle_start_time();

    debug_task.execute_on_time(control_cycle_start);
    mission_manager.execute_on_time(control_cycle_start);
    docking_controller.execute_on_time(control_cycle_start);
}
