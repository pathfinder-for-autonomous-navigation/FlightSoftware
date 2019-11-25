#include "MainControlLoop.hpp"
#include "DebugTask.hpp"

// Environment-based initializations of the control loop time.
#ifdef HOOTL
    #ifdef DESKTOP
        static constexpr unsigned int control_cycle_time = 170000000;
    #else
        static constexpr unsigned int control_cycle_time = 170000;
    #endif
#elif FLIGHT
    static constexpr unsigned int control_cycle_time = 120000;
#endif

#ifdef DESKTOP
    #define PIKSI_INITIALIZATION piksi("piksi")
#else
    #include <HardwareSerial.h>
    #define PIKSI_INITIALIZATION piksi("piksi", Serial4)
#endif

MainControlLoop::MainControlLoop(StateFieldRegistry& registry)
    : ControlTask<void>(registry), 
      field_creator_task(registry),
      clock_manager(registry, control_cycle_time),
      debug_task(registry, debug_task_offset),
      PIKSI_INITIALIZATION,
      piksi_control_task(registry, piksi_control_task_offset, piksi),
      mission_manager(registry, mission_manager_offset),
      docksys(),
      docking_controller(registry, docking_controller_offset, docksys),
      quake_manager(registry, quake_manager_offset)
{}

void MainControlLoop::execute() {
    clock_manager.execute();
    sys_time_t control_cycle_start = clock_manager.get_control_cycle_start_time();

    #ifdef HOOTL
    debug_task.execute_on_time(control_cycle_start);
    #endif

    piksi_control_task.execute_on_time(control_cycle_start);
    mission_manager.execute_on_time(control_cycle_start);
    quake_manager.execute_on_time(control_cycle_start);
    docking_controller.execute_on_time(control_cycle_start);
}
