#include "MainControlLoop.hpp"
#include "DebugTask.hpp"
#include "constants.hpp"

// Include for calculating memory use. Works only on Mac and Linux.
#ifdef DESKTOP
    #include <unistd.h>
#endif

#ifdef DESKTOP
    #define PIKSI_INITIALIZATION piksi("piksi")
#else
    #include <HardwareSerial.h>
    #define PIKSI_INITIALIZATION piksi("piksi", Serial4)
#endif

MainControlLoop::MainControlLoop(StateFieldRegistry& registry,
        const std::vector<DownlinkProducer::FlowData>& flow_data)
    : ControlTask<void>(registry), 
      field_creator_task(registry),
      clock_manager(registry, PAN::control_cycle_time),
      debug_task(registry, debug_task_offset),
      PIKSI_INITIALIZATION,
      piksi_control_task(registry, piksi_control_task_offset, piksi),
      attitude_estimator(registry, attitude_estimator_offset),
      gomspace(&hk, &config, &config2),
      gomspace_controller(registry, gomspace_controller_offset, gomspace),
      mission_manager(registry, mission_manager_offset),
      docksys(),
      docking_controller(registry, docking_controller_offset, docksys),
      downlink_producer(registry, downlink_producer_offset, flow_data),
      quake_manager(registry, quake_manager_offset),
      memory_use_f("sys.memory_use", Serializer<unsigned int>())
{
    #ifdef FUNCTIONAL_TEST
        add_readable_field(memory_use_f);
    #endif
}

#ifndef DESKTOP
// Taken from SdFatUtil.h
static int FreeRam(void) {
 extern int  __bss_end;
 extern int* __brkval;
 int free_memory;
 if (reinterpret_cast<int>(__brkval) == 0) {
   // if no heap use from end of bss section
   free_memory = reinterpret_cast<int>(&free_memory)
                 - reinterpret_cast<int>(&__bss_end);
 } else {
   // use from top of stack to heap
   free_memory = reinterpret_cast<int>(&free_memory)
                 - reinterpret_cast<int>(__brkval);
 }
 return free_memory;
}
#endif

void MainControlLoop::execute() {
    // Compute memory usage
    #ifdef DESKTOP
        unsigned int pages = sysconf(_SC_PHYS_PAGES);
        unsigned int page_size = sysconf(_SC_PAGE_SIZE);
        memory_use_f.set(pages * page_size);
    #else
        memory_use_f.set(FreeRam());
    #endif

    clock_manager.execute();

    #ifdef FUNCTIONAL_TEST
    debug_task.execute_on_time();
    #endif

    piksi_control_task.execute_on_time();
    mission_manager.execute_on_time();
    downlink_producer.execute_on_time();
    quake_manager.execute_on_time();
    docking_controller.execute_on_time();
}

#ifdef GSW
DownlinkProducer* MainControlLoop::get_downlink_producer() {
    return &downlink_producer;
}
#endif
