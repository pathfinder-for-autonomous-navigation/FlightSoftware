#include "MainControlLoop.hpp"
#include "DebugTask.hpp"
#include "constants.hpp"
#include <common/constant_tracker.hpp>

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
    TRACKED_CONSTANT_S(HardwareSerial&, piksi_serial, Serial4);
    #define PIKSI_INITIALIZATION piksi("piksi", piksi_serial)
    #define ADCS_INITIALIZATION adcs(Wire, Devices::ADCS::ADDRESS)
#endif

MainControlLoop::MainControlLoop(StateFieldRegistry& registry,
        const std::vector<DownlinkProducer::FlowData>& flow_data)
    : ControlTask<void>(registry),
      field_creator_task(registry),
      clock_manager(registry, PAN::control_cycle_time),
      PIKSI_INITIALIZATION,
      piksi_control_task(registry, piksi_control_task_offset, piksi),
      ADCS_INITIALIZATION,
      adcs_monitor(registry, adcs_monitor_offset, adcs),
      debug_task(registry, debug_task_offset),
      attitude_estimator(registry, attitude_estimator_offset),
      gomspace(&hk, &config, &config2),
      gomspace_controller(registry, gomspace_controller_offset, gomspace),
      docksys(),
      docking_controller(registry, docking_controller_offset, docksys),
      downlink_producer(registry, downlink_producer_offset),
      quake_manager(registry, quake_manager_offset),
      uplink_consumer(registry, uplink_consumer_offset),
      dcdc("dcdc"),
      dcdc_controller(registry, dcdc_controller_offset, dcdc),
      eeprom_controller(registry, eeprom_controller_offset),
      memory_use_f("sys.memory_use", Serializer<unsigned int>(300000)),
      mission_manager(registry, mission_manager_offset), // This item is initialized near-last so it has access to all state fields
      attitude_computer(registry, attitude_computer_offset), // This item needs "adcs.state" from mission manager.
      adcs_commander(registry, adcs_commander_offset), // needs inputs from attitude computer
      adcs_box_controller(registry, adcs_box_controller_offset, adcs)
{
    docking_controller.init();

    //setup I2C bus for Flight Controller
    #ifndef DESKTOP
    TRACKED_CONSTANT_SC(i2c_mode, i2c_mode_sel, I2C_MASTER);
    TRACKED_CONSTANT_SC(i2c_pins, i2c_pin_nos, I2C_PINS_18_19);
    TRACKED_CONSTANT_SC(i2c_pullup, i2c_pullups, I2C_PULLUP_EXT);
    TRACKED_CONSTANT_SC(unsigned int, i2c_rate, 400000);
    TRACKED_CONSTANT_SC(i2c_op_mode, i2c_op, I2C_OP_MODE_IMM);
    Wire.begin(i2c_mode_sel, 0x00, i2c_pin_nos, i2c_pullups, i2c_rate, i2c_op);
    #endif
    
    //setup I2C devices
    adcs.setup();
    gomspace.setup();
    dcdc.setup();

    #ifdef FUNCTIONAL_TEST
        add_readable_field(memory_use_f);
    #endif

    eeprom_controller.init();
    // Since all telemetry fields have been added to the registry, initialize flows
    downlink_producer.init_flows(flow_data);

    // Temporarily disable fault handling until it's better tested
    WritableStateField<bool>* fault_handler_enabled_fp =
        find_writable_field<bool>("fault_handler.enabled", __FILE__, __LINE__);
    fault_handler_enabled_fp->set(false);
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

    piksi_control_task.execute_on_time();
    gomspace_controller.execute_on_time();
    adcs_monitor.execute_on_time();

    #ifdef FUNCTIONAL_TEST
    debug_task.execute_on_time();
    #endif

    attitude_estimator.execute_on_time();
    mission_manager.execute_on_time();
    attitude_computer.execute_on_time();
    adcs_commander.execute_on_time();
    adcs_box_controller.execute_on_time();
    downlink_producer.execute_on_time();
    quake_manager.execute_on_time();
    docking_controller.execute_on_time();
    dcdc_controller.execute_on_time();
    
    #ifdef DESKTOP
        eeprom_controller.execute_on_time();
    #else
        // eeprom_controller.execute_on_time();
        // Commented to save EEPROM Cycles
    #endif
}

#ifdef GSW
DownlinkProducer* MainControlLoop::get_downlink_producer() {
    return &downlink_producer;
}
#endif
