/**
 * @file hardware_setup.cpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @brief Contains implementation for hardware setup function.
 * @date 2019-05-27
 */

#include <ChRt.h>
#include <chibi_helpers.hpp>
#include "state/state_holder.hpp"
#include "startup.hpp"
#include "debug.hpp"

#define DEVICE_HEAP_SIZE 4096
memory_heap_t device_heap;
static CH_HEAP_AREA(device_heap_area, DEVICE_HEAP_SIZE);

// Statically-allocated metadata for HAT
static State::Hardware::DeviceState adcs_device_state = {false, false, false, false, 1};
static State::Hardware::DeviceState dcdc_device_state = {false, false, false, false, 1};
static State::Hardware::DeviceState docking_motor_device_state = {false, false, false, false, 1};
static State::Hardware::DeviceState docking_switch_device_state = {false, false, false, false, 1};
static State::Hardware::DeviceState gomspace_device_state = {false, false, false, false, 1};
static State::Hardware::DeviceState piksi_device_state = {false, false, false, false, 1};
static State::Hardware::DeviceState system_output_device_state = {false, false, false, false, 1};
static State::Hardware::DeviceState pressure_sensor_device_state = {false, false, false, false, 1};
static State::Hardware::DeviceState quake_device_state = {false, false, false, false, 1};
static State::Hardware::DeviceState sph_device_state = {false, false, false, false, 1};
static State::Hardware::DeviceState temp_sensor_inner_device_state = {false, false, false, false, 1};
static State::Hardware::DeviceState temp_sensor_outer_device_state = {false, false, false, false, 1};
static std::string adcs_system_name = "adcs_system";
static std::string dcdc_device_name = "dcdc";
static std::string docking_motor_device_name = "docking_motor";
static std::string docking_switch_device_name = "docking_switch";
static std::string gomspace_device_name = "gomspace";
static std::string fake_piksi_device_name = "fake_piksi"; // static std::string piksi_device_name = "piksi";
static std::string system_output_device_name = "system_output";
static std::string pressure_sensor_device_name = "pressure_sensor";
static std::string quake_device_name = "quake";
static std::string sph_device_name = "spike_and_hold";
static std::string temp_sensor_inner_device_name = "temp_sensor_inner";
static std::string temp_sensor_outer_device_name = "temp_sensor_outer";

void hardware_setup() {
    rwMtxObjectInit(&State::Hardware::hardware_state_lock);

    dbg.println(debug_severity::INFO, "Initializing logical device objects.");
    chHeapObjectInit(&device_heap, device_heap_area, DEVICE_HEAP_SIZE);

    // Devices::gomspace = new(device_heap) Devices::Gomspace(gomspace_device_name,
    //                             &State::Gomspace::gomspace_data,
    //                             &State::Gomspace::gomspace_config,
    //                             &State::Gomspace::gomspace_config2, 
    //                             Wire, Devices::Gomspace::ADDRESS);
    // Devices::spike_and_hold = new(device_heap) Devices::SpikeAndHold(sph_device_name, 
    //                                   Devices::SpikeAndHold::DEFAULT_VALVE_PINS, 
    //                                   Devices::SpikeAndHold::DEFAULT_ENABLE_PIN);
    Devices::system_output = new(device_heap) Devices::SystemOutput(system_output_device_name, Serial3);
    Devices::piksi    = new(device_heap) Devices::FakePiksi(fake_piksi_device_name, Serial4); // Devices::Piksi(piksi_device_name, Serial4);
    // Devices::dcdc  = new(device_heap) Devices::DCDC(dcdc_device_name, Devices::DCDC::DEFAULT_ENABLE_PIN);
    // Devices::quake = new(device_heap) Devices::QLocate(quake_device_name, &Serial3, Devices::QLocate::DEFAULT_NR_PIN, 
    //                          Devices::QLocate::DEFAULT_TIMEOUT);
    // Devices::adcs_system = new(device_heap) Devices::ADCS(adcs_system_device_name, Wire, Devices::ADCS::ADDRESS);
    // Devices::pressure_sensor = new(device_heap) Devices::PressureSensor("pressure_sensor",
    //                                    Devices::PressureSensor::DEFAULT_LOW_PRESSURE_PIN,
    //                                    Devices::PressureSensor::DEFAULT_HIGH_PRESSURE_PIN);
    // Devices::temp_sensor_inner = new(device_heap) Devices::TempSensor(temp_sensor_inner_device_name, 
    //                                      Devices::TempSensor::DEFAULT_PIN_INNER_TANK);
    // Devices::temp_sensor_inner = new(device_heap) Devices::TempSensor(temp_sensor_outer_device_name, 
    //                                      Devices::TempSensor::DEFAULT_PIN_OUTER_TANK);
    // Devices::docking_motor  = new(device_heap) Devices::DockingMotor(docking_motor_device_name, 
    //                                  Devices::DockingMotor::DEFAULT_MOTOR_I1_PIN,
    //                                  Devices::DockingMotor::DEFAULT_MOTOR_I2_PIN,
    //                                  Devices::DockingMotor::DEFAULT_MOTOR_DIRECTION_PIN,
    //                                  Devices::DockingMotor::DEFAULT_MOTOR_SLEEP_PIN,
    //                                  Devices::DockingMotor::DEFAULT_MOTOR_STEP_PIN);
    // Devices::docking_switch = new(device_heap) Devices::DockingSwitch(docking_switch_device_name, 
    //                                  Devices::DockingSwitch::DEFAULT_SWITCH_PIN);

    dbg.println(debug_severity::INFO, "Adding devices to HAT.");
    // State::Hardware::hat.insert({Devices::gomspace, gomspace_device_state});
    // State::Hardware::hat.insert({Devices::spike_and_hold, sph_device_state});
    State::Hardware::hat.insert({Devices::system_output, system_output_device_state});
    State::Hardware::hat.insert({Devices::piksi, piksi_device_state});
    // State::Hardware::hat.insert({Devices::dcdc, dcdc_device_state});
    // State::Hardware::hat.insert({Devices::quake, quake_device_state});
    // State::Hardware::hat.insert({Devices::adcs_system, adcs_device_state});
    // State::Hardware::hat.insert({Devices::pressure_sensor, pressure_sensor_device_state});
    // State::Hardware::hat.insert({Devices::temp_sensor_inner, temp_sensor_inner_device_state});
    // State::Hardware::hat.insert({Devices::temp_sensor_outer, temp_sensor_outer_device_state});
    // State::Hardware::hat.insert({Devices::docking_motor, docking_motor_device_state});
    // State::Hardware::hat.insert({Devices::docking_switch, docking_switch_device_state});

    dbg.println(debug_severity::INFO, "Initializing hardware buses.");
    // Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM); // Gomspace

    dbg.println(debug_severity::INFO, "Setting up hardware.");
    for (auto device : State::Hardware::hat)
    {
        Devices::Device *dptr = device.first;
        const char* dev_name = dptr->name().c_str();
        dptr->setup();
        if (dptr->is_functional())
        {
            dbg.printf(debug_severity::INFO, "Setup of %s: successful!", dev_name);
            State::write((State::Hardware::hat).at(device.first).powered_on, true, State::Hardware::hardware_state_lock);
            State::write((State::Hardware::hat).at(device.first).is_functional, true, State::Hardware::hardware_state_lock);
        }
        else
            dbg.printf(debug_severity::INFO, "Setup of %s: unsuccessful.", dev_name);
    }
}