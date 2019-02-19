/** @file device_declarations.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains initializations for device objects representing devices in the satellite.
 */

#include <i2c_t3.h>
#include "device_states.hpp"
#include "state_holder.hpp"

namespace Devices {
    Gomspace gomspace(&State::Gomspace::gomspace_data,
                      &State::Gomspace::gomspace_config,
                      &State::Gomspace::gomspace_config2, 
                      Wire, Gomspace::ADDRESS);
    QLocate quake(&Serial3, 20);
    Piksi piksi(Serial4);
    DCDC dcdc(DCDC::DEFAULT_ENABLE_PIN);
    SpikeAndHold spike_and_hold(SpikeAndHold::DEFAULT_VALVE_PINS, 
                                SpikeAndHold::DEFAULT_ENABLE_PIN);
    ADCS adcs_system(Wire, ADCS::ADDRESS);
    PressureSensor pressure_sensor(PressureSensor::DEFAULT_LOW_PRESSURE_PIN,
                                   PressureSensor::DEFAULT_HIGH_PRESSURE_PIN);
    TempSensor temp_sensor_inner(TempSensor::DEFAULT_PIN_INNER_TANK);
    TempSensor temp_sensor_outer(TempSensor::DEFAULT_PIN_OUTER_TANK);
    DockingSwitch docking_switch(DockingSwitch::DEFAULT_SWITCH_PIN);
    DockingMotor docking_motor(DockingMotor::DEFAULT_MOTOR_I1_PIN,
                               DockingMotor::DEFAULT_MOTOR_I2_PIN,
                               DockingMotor::DEFAULT_MOTOR_DIRECTION_PIN,
                               DockingMotor::DEFAULT_MOTOR_SLEEP_PIN,
                               DockingMotor::DEFAULT_MOTOR_RESET_PIN,
                               DockingMotor::DEFAULT_MOTOR_STEP_PIN);
}

namespace State {
namespace Hardware {
    std::map<std::string, Devices::Device&> devices = {
        {Devices::gomspace.name(), Devices::gomspace},
        {Devices::piksi.name(), Devices::piksi},
        {Devices::spike_and_hold.name(), Devices::spike_and_hold},
        {Devices::dcdc.name(), Devices::dcdc},
        {Devices::adcs_system.name(), Devices::adcs_system},
        {Devices::quake.name(), Devices::quake},
        {Devices::pressure_sensor.name(), Devices::pressure_sensor},
        {Devices::temp_sensor_inner.name(), Devices::temp_sensor_inner},
        {Devices::temp_sensor_outer.name(), Devices::temp_sensor_outer},
        {Devices::docking_switch.name(), Devices::docking_switch},
        {Devices::docking_motor.name(), Devices::docking_motor}
    };
    static Hardware::DeviceState gomspace_device_state = {false, false, false};
    static Hardware::DeviceState piksi_device_state = {false, false, false};
    static Hardware::DeviceState sph_device_state = {false, false, false};
    static Hardware::DeviceState dcdc_device_state = {false, false, false};
    static Hardware::DeviceState adcs_device_state = {false, false, false};
    static Hardware::DeviceState quake_device_state = {false, false, false};
    static Hardware::DeviceState prop_pressure_device_state = {false, false, false};
    static Hardware::DeviceState prop_temp_inner_device_state = {false, false, false};
    static Hardware::DeviceState prop_temp_outer_device_state = {false, false, false};
    static Hardware::DeviceState docking_switch_device_state = {false, false, false};
    static Hardware::DeviceState docking_motor_device_state = {false, false, false};
    std::map<std::string, Hardware::DeviceState&> hat {
        {Devices::gomspace.name(), gomspace_device_state},
        {Devices::piksi.name(), piksi_device_state},
        {Devices::spike_and_hold.name(), sph_device_state},
        {Devices::dcdc.name(), dcdc_device_state},
        {Devices::adcs_system.name(), adcs_device_state},
        {Devices::quake.name(), quake_device_state},
        {Devices::pressure_sensor.name(), prop_pressure_device_state},
        {Devices::temp_sensor_inner.name(), prop_temp_inner_device_state},
        {Devices::temp_sensor_outer.name(), prop_temp_outer_device_state},
        {Devices::docking_switch.name(), docking_switch_device_state},
        {Devices::docking_motor.name(), docking_motor_device_state}
    };
    bool is_hardware_setup = false;
    rwmutex_t hat_lock;

    mutex_t dcdc_lock;
    mutex_t adcs_device_lock;
    mutex_t spike_and_hold_lock;
    mutex_t piksi_device_lock;
    mutex_t quake_device_lock;
    mutex_t gomspace_device_lock;
}
}