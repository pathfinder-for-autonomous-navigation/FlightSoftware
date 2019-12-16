#include "MainControlLoop.hpp"
#include "DebugTask.hpp"
#include "constants.hpp"

// Include for calculating memory use.
#ifdef DESKTOP
    #include <memuse.h>
#else
    extern "C" char* sbrk(int incr);
#endif

#ifdef DESKTOP
    #define PIKSI_INITIALIZATION piksi("piksi")
    #define ADCS_INITIALIZATION adcs()
#else
    #include <HardwareSerial.h>
    #define PIKSI_INITIALIZATION piksi("piksi", Serial4)
    #define ADCS_INITIALIZATION adcs(Wire, Devices::ADCS::ADDRESS)
#endif

MainControlLoop::MainControlLoop(StateFieldRegistry& registry,
        const std::vector<DownlinkProducer::FlowData>& flow_data)
    : ControlTask<void>(registry), 
      field_creator_task(registry),
      clock_manager(registry, PAN::control_cycle_time),
      debug_task(registry, debug_task_offset),
      PIKSI_INITIALIZATION,
      piksi_control_task(registry, piksi_control_task_offset, piksi),
      ADCS_INITIALIZATION,
      adcs_monitor(registry, adcs_monitor_offset, adcs),
      attitude_estimator(registry, attitude_estimator_offset),
      gomspace(&hk, &config, &config2),
      gomspace_controller(registry, gomspace_controller_offset, gomspace),
      mission_manager(registry, mission_manager_offset),
      docksys(),
      docking_controller(registry, docking_controller_offset, docksys),
      downlink_producer(registry, downlink_producer_offset, flow_data),
      quake_manager(registry, quake_manager_offset),
      memory_use_f("sys.memory_use", Serializer<unsigned int>(300000))
{
    //setup I2C bus for Flight Controller
    #ifndef DESKTOP
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
    #endif
    
    //setup I2C devices
    adcs.setup();
    gomspace.setup();

    #ifdef FUNCTIONAL_TEST
        add_readable_field(memory_use_f);
    #endif
}

void MainControlLoop::execute() {
    // Compute memory usage
    #ifdef DESKTOP
    memory_use_f.set(getCurrentRSS());
    #else
    char top;
    memory_use_f.set(&top - reinterpret_cast<char*>(sbrk(0)));
    #endif

    clock_manager.execute();

    #ifdef FUNCTIONAL_TEST
    debug_task.execute_on_time();
    #endif

    piksi_control_task.execute_on_time();
    gomspace_controller.execute_on_time();
    attitude_estimator.execute_on_time();
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
