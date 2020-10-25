/**
 * @file mtr_ptest.cpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * 
 * Simplified version of flight software that can be used in
 * conjunction with PTest for high-rate sensor and actuator
 * sampling.
 */

#include <fsw/FCCode/ClockManager.hpp>
#include <fsw/FCCode/ADCSBoxMonitor.hpp>
#include <fsw/FCCode/AttitudeEstimator.hpp>
#include <fsw/FCCode/AttitudeComputer.hpp>
#include <fsw/FCCode/DCDCController.hpp>
#include <fsw/FCCode/DebugTask.hpp>
#include <fsw/FCCode/ADCSBoxController.hpp>
#include <fsw/FCCode/ADCSCommander.hpp>
#include <fsw/FCCode/Drivers/Gomspace.hpp>
#include <common/StateFieldRegistry.hpp>

#ifndef UNIT_TEST

class MTRTestControlLoop : public ControlTask<void> {
  private:
    #ifdef DESKTOP
        #define ADCS_INITIALIZATION adcs()
        #define I2C_INITIALIZATION
    #else
        #define ADCS_INITIALIZATION adcs(Wire, Devices::ADCS::ADDRESS)
        #define I2C_INITIALIZATION \
          static constexpr i2c_mode i2c_mode_sel {I2C_MASTER}; \
          static constexpr i2c_pins i2c_pin_nos {I2C_PINS_18_19}; \
          static constexpr i2c_pullup i2c_pullups {I2C_PULLUP_EXT}; \
          static constexpr unsigned int i2c_rate = 400000; \
          static constexpr i2c_op_mode i2c_op {I2C_OP_MODE_IMM}; \
          Wire.begin(i2c_mode_sel, 0x00, i2c_pin_nos, i2c_pullups, i2c_rate, i2c_op);
    #endif

    /**
     * A special field creator task for the MTR PTest case.
     */
    class FieldCreator {
      private:
        WritableStateField<unsigned char> adcs_state_f;
        ReadableStateField<double> time_f;
        ReadableStateField<lin::Vector3d> pos_f;
        ReadableStateField<lin::Vector3d> pos_baseline_f;

      public:
        FieldCreator(StateFieldRegistry& r) :
            adcs_state_f("adcs.state", Serializer<unsigned char>()),
            time_f("orbit.time", Serializer<double>(0.0, 18'446'744'073'709'551'616.0, 64)),
            pos_f("orbit.pos", Serializer<lin::Vector3d>(0,100000,100)),
            pos_baseline_f("orbit.baseline_pos", Serializer<lin::Vector3d>(0,100000,100))
        {
            r.add_writable_field(&adcs_state_f);
            r.add_readable_field(&time_f);
            r.add_readable_field(&pos_f);
            r.add_readable_field(&pos_baseline_f);
        }
    };

    /**
     * Driver instantiations for hardware devices.
     */
    Devices::ADCS adcs;
    Devices::DCDC dcdc;
    Devices::Gomspace::eps_hk_t hk;
    Devices::Gomspace::eps_config_t config; 
    Devices::Gomspace::eps_config2_t config2; 
    Devices::Gomspace gomspace;

    /**
     * Control tasks required by the MTR ptest case.
     * 
     * Only some of these control tasks are actually run
     * by the control loop. We instantiate the others so
     * that they can instantiate state fields expected by
     * the tasks required by the control loop. The unnecessary
     * control tasks are marked as such.
     */
    FieldCreator field_creator_mtr;
    ClockManager clock_manager;
    ADCSBoxMonitor adcs_monitor;
    AttitudeEstimator attitude_estimator; // This control task is only used to initialize state fields.
    AttitudeComputer attitude_computer; // This control task is only used to initialize state fields.
    ADCSCommander adcs_commander; // This control task is only used to initialize state fields.
    DCDCController dcdc_controller;
    DebugTask debug_task;
    ADCSBoxController adcs_box_controller;

    /**
     * These fields are used by PTest to determine runtime
     * characteristics of the flight software. Their values
     * aren't really important but they must exist.
     */
    ReadableStateField<unsigned int> one_day_ccno_f;
    ReadableStateField<unsigned int> control_cycle_ms_f;

    /**
     * Control cycle runtime constants.
     */
    static constexpr unsigned int control_cycle_time = 20000;
    static constexpr unsigned int dcdc_controller_offset = 0;
    static constexpr unsigned int adcs_monitor_offset = 500;
    static constexpr unsigned int debug_task_offset = 5000;
    static constexpr unsigned int adcs_box_controller_offset = 10000;

  public:
    MTRTestControlLoop(StateFieldRegistry& registry)
    : ControlTask<void>(registry),
      ADCS_INITIALIZATION,
      dcdc("dcdc"),
      gomspace(&hk, &config, &config2),
      field_creator_mtr(registry),
      clock_manager(registry, control_cycle_time),
      adcs_monitor(registry, adcs_monitor_offset, adcs),
      attitude_estimator(registry, 0),
      attitude_computer(registry, 0),
      adcs_commander(registry, 0),
      dcdc_controller(registry, dcdc_controller_offset, dcdc),
      debug_task(registry, debug_task_offset),
      adcs_box_controller(registry, adcs_box_controller_offset, adcs),
      one_day_ccno_f("pan.one_day_ccno", Serializer<unsigned int>()),
      control_cycle_ms_f("pan.cc_ms", Serializer<unsigned int>())
    {
        //setup I2C bus for Flight Controller
        I2C_INITIALIZATION;

        //setup I2C devices
        adcs.setup();
        gomspace.setup();
        dcdc.setup();

        registry.add_readable_field(&control_cycle_ms_f);
        control_cycle_ms_f.set(control_cycle_time / 1000);
        registry.add_readable_field(&one_day_ccno_f);
        one_day_ccno_f.set(24 * 60 * 60 * 1000 / control_cycle_ms_f.get());

        #ifndef DESKTOP
            // Allow for PTest testing to occur in a controlled way.
            while(!Serial) {}
        #endif
    }

    void execute() {
        gomspace.get_hk();
        clock_manager.execute();
        dcdc_controller.execute_on_time();
        adcs_monitor.execute_on_time();
        debug_task.execute_on_time();
        adcs_box_controller.execute_on_time();
    }
};

int main() {
    StateFieldRegistry registry;
    MTRTestControlLoop fcp(registry);

    while (true) {
        fcp.execute();
    }
    return 0;
}
#endif
