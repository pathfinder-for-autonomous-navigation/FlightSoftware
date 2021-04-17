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
      piksi_control_task(registry, piksi),
      ADCS_INITIALIZATION,
      adcs_monitor(registry, adcs),
      debug_task(registry),
       estimators(registry),
      gomspace(&hk, &config, &config2),
      gomspace_controller(registry, gomspace),
      docksys(),
      docking_controller(registry, docksys),
      downlink_producer(registry),
      quake_manager(registry),
      uplink_consumer(registry),
      dcdc("dcdc"),
      dcdc_controller(registry, dcdc),
      eeprom_controller(registry),
      memory_use_f("sys.memory_use", Serializer<unsigned int>(300000)),
      one_day_ccno_f("pan.one_day_ccno", Serializer<unsigned int>()),
      control_cycle_ms_f("pan.cc_ms", Serializer<unsigned int>()),
      prop_controller(registry),
      mission_manager(registry), // This item is initialized near-last so it has access to all state fields
      attitude_controller(registry),
      adcs_commander(registry), // needs inputs from attitude computer
      adcs_box_controller(registry, adcs),
      orbit_controller(registry)
{
    
    docking_controller.init();
    orbit_controller.init();

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

    add_readable_field(memory_use_f);
    add_readable_field(one_day_ccno_f);
    add_readable_field(control_cycle_ms_f);
    one_day_ccno_f.set(PAN::one_day_ccno);
    control_cycle_ms_f.set(PAN::control_cycle_time_ms);

    eeprom_controller.init();
    // Since all telemetry fields have been added to the registry, initialize flows
    downlink_producer.init_flows(flow_data);
    
    // grab downlink sizes, intialize MO buffers
    quake_manager.init();
    
    #ifndef FLIGHT
    #ifndef DESKTOP
        // Allow for PTest testing to occur in a controlled way.
        while(!Serial) {}
    #endif
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

    TRACKED_CONSTANT_SC(unsigned int, piksi_duration, 6400);
    TRACKED_CONSTANT_SC(unsigned int, adcs_monitor_duration, 28000);
    TRACKED_CONSTANT_SC(unsigned int, debug_duration, 50000);
    TRACKED_CONSTANT_SC(unsigned int, gomspace_duration, 15000);
    TRACKED_CONSTANT_SC(unsigned int, uplink_duration, 10000);
    TRACKED_CONSTANT_SC(unsigned int, attitude_estimator_duration, 5000);
    TRACKED_CONSTANT_SC(unsigned int, mission_duration, 1000);
    TRACKED_CONSTANT_SC(unsigned int, dcdc_duration, 1000);
    TRACKED_CONSTANT_SC(unsigned int, attitude_controller_duration, 1000);
    TRACKED_CONSTANT_SC(unsigned int, adcs_commander_duration, 1000);
    TRACKED_CONSTANT_SC(unsigned int, adcs_box_controller_duration, 10000);
    TRACKED_CONSTANT_SC(unsigned int, orbit_duration, 5000);
    TRACKED_CONSTANT_SC(unsigned int, prop_duration, 1000);
    TRACKED_CONSTANT_SC(unsigned int, downlink_duration, 1000);
    TRACKED_CONSTANT_SC(unsigned int, quake_duration, 30000);
    TRACKED_CONSTANT_SC(unsigned int, docking_duration, 10000);
    TRACKED_CONSTANT_SC(unsigned int, eeprom_duration, 10000);

    clock_manager.execute();

    piksi_control_task.execute_on_time(piksi_duration);
    gomspace_controller.execute_on_time(gomspace_duration);
    adcs_monitor.execute_on_time(adcs_monitor_duration);
    
    debug_task.execute_on_time(debug_duration);

    uplink_consumer.execute_on_time(uplink_duration);
    estimators.execute_on_time(attitude_estimator_duration);
    mission_manager.execute_on_time(mission_duration);
    dcdc_controller.execute_on_time(dcdc_duration);
    attitude_controller.execute_on_time(attitude_controller_duration);
    adcs_commander.execute_on_time(adcs_commander_duration);
    adcs_box_controller.execute_on_time(adcs_box_controller_duration);
    orbit_controller.execute_on_time(orbit_duration);
    prop_controller.execute_on_time(prop_duration);
    downlink_producer.execute_on_time(downlink_duration);
    quake_manager.execute_on_time(quake_duration);
    docking_controller.execute_on_time(docking_duration);
    
    #ifdef DESKTOP
        eeprom_controller.execute_on_time(eeprom_duration);
    #else
        eeprom_controller.execute_on_time(eeprom_duration);
        // Commented to save EEPROM Cycles
    #endif
}

#ifdef GSW
DownlinkProducer* MainControlLoop::get_downlink_producer() {
    return &downlink_producer;
}
#endif
