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

MainControlLoop::MainControlLoop(StateFieldRegistry& registry,
        const std::vector<DownlinkProducer::FlowData>& flow_data)
    : ControlTask<void>(registry), 
      field_creator_task(registry),
      clock_manager(registry, control_cycle_time),
      debug_task(registry, debug_task_offset),
      mission_manager(registry, mission_manager_offset),
      docksys(),
      docking_controller(registry, docking_controller_offset, docksys),
      downlink_producer(registry, downlink_producer_offset, flow_data)
{}

void MainControlLoop::execute() {
    clock_manager.execute();
    sys_time_t control_cycle_start = clock_manager.get_control_cycle_start_time();

    #ifdef HOOTL
    debug_task.execute_on_time(control_cycle_start);
    #endif

    mission_manager.execute_on_time(control_cycle_start);
    docking_controller.execute_on_time(control_cycle_start);
}

#ifdef GSW
DownlinkProducer* MainControlLoop::get_downlink_producer() {
    return &downlink_producer;
}
#endif
