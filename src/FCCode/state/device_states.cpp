/** @file device_declarations.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains initializations for device objects representing devices in the satellite.
 */

#include <i2c_t3.h>
#include "device_states.hpp"
#include <EEPROM.h>
#include "EEPROMAddresses.hpp"
#include "state_holder.hpp"

namespace Devices {
    static std::string gomspace_name = "gomspace";
    static std::string quake_name = "quake";
    static std::string piksi_name = "piksi";
    static std::string dcdc_name = "dcdc";
    static std::string spike_and_hold_name = "spike_and_hold";
    static std::string adcs_system_name = "adcs_system";
    static std::string pressure_sensor_name = "pressure_sensor";
    static std::string temp_sensor_inner_name = "temp_sensor_inner";
    static std::string temp_sensor_outer_name = "temp_sensor_outer";
    static std::string docking_switch_name = "docking_switch";
    static std::string docking_motor_name = "docking_motor";

    Gomspace& gomspace() {
        static Gomspace g(gomspace_name,
                          &State::Gomspace::gomspace_data,
                          &State::Gomspace::gomspace_config,
                          &State::Gomspace::gomspace_config2, 
                          Wire, Gomspace::ADDRESS);
        return g;
    }

    Piksi& piksi() {
        static Piksi p(piksi_name, Serial4);
        return p;
    }
    SpikeAndHold& spike_and_hold() {
        static SpikeAndHold s(spike_and_hold_name, 
                              SpikeAndHold::DEFAULT_VALVE_PINS, 
                              SpikeAndHold::DEFAULT_ENABLE_PIN);
        return s;
    }
    DCDC& dcdc() {
        static DCDC d(dcdc_name, DCDC::DEFAULT_ENABLE_PIN);
        return d;
    }
    QLocate& quake() {
        static QLocate q(quake_name, &Serial3, QLocate::DEFAULT_NR_PIN, QLocate::DEFAULT_TIMEOUT);
        return q;
    }
    ADCS& adcs_system() {
        static ADCS a(adcs_system_name, Wire, ADCS::ADDRESS);
        return a;
    }
    PressureSensor& pressure_sensor() {
        static PressureSensor p(pressure_sensor_name,
                                    PressureSensor::DEFAULT_LOW_PRESSURE_PIN,
                                    PressureSensor::DEFAULT_HIGH_PRESSURE_PIN);
        return p;
    }
    TempSensor& temp_sensor_inner() {
        static TempSensor t(temp_sensor_inner_name, TempSensor::DEFAULT_PIN_INNER_TANK);
        return t;
    }
    TempSensor& temp_sensor_outer() {
        static TempSensor t(temp_sensor_outer_name, TempSensor::DEFAULT_PIN_OUTER_TANK);
        return t;
    }
    DockingMotor& docking_motor() {
        static DockingMotor d(docking_motor_name, 
                              DockingMotor::DEFAULT_MOTOR_I1_PIN,
                              DockingMotor::DEFAULT_MOTOR_I2_PIN,
                              DockingMotor::DEFAULT_MOTOR_DIRECTION_PIN,
                              DockingMotor::DEFAULT_MOTOR_SLEEP_PIN,
                              DockingMotor::DEFAULT_MOTOR_STEP_PIN);
        return d;
    }
    DockingSwitch& docking_switch() {
        static DockingSwitch d(docking_switch_name, DockingSwitch::DEFAULT_SWITCH_PIN);
        return d;
    }
}

namespace State {
namespace Hardware {
    static Hardware::DeviceState adcs_device_state = {false, false, false, false, 1};
    static Hardware::DeviceState dcdc_device_state = {false, false, false, false, 1};
    static Hardware::DeviceState docking_motor_device_state = {false, false, false, false, 1};
    static Hardware::DeviceState docking_switch_device_state = {false, false, false, false, 1};
    static Hardware::DeviceState gomspace_device_state = {false, false, false, false, 1};
    static Hardware::DeviceState piksi_device_state = {false, false, false, false, 1};
    static Hardware::DeviceState pressure_sensor_device_state = {false, false, false, false, 1};
    static Hardware::DeviceState quake_device_state = {false, false, false, false, 1};
    static Hardware::DeviceState sph_device_state = {false, false, false, false, 1};
    static Hardware::DeviceState temp_sensor_inner_device_state = {false, false, false, false, 1};
    static Hardware::DeviceState temp_sensor_outer_device_state = {false, false, false, false, 1};
    std::map<Devices::Device*, Hardware::DeviceState> hat {
        {&Devices::adcs_system(), adcs_device_state},
        {&Devices::dcdc(), dcdc_device_state},
        {&Devices::docking_motor(), docking_motor_device_state},
        {&Devices::docking_switch(), docking_switch_device_state},
        {&Devices::gomspace(), gomspace_device_state},
        {&Devices::piksi(), piksi_device_state},
        {&Devices::pressure_sensor(), pressure_sensor_device_state},
        {&Devices::quake(), quake_device_state},
        {&Devices::spike_and_hold(), sph_device_state},
        {&Devices::temp_sensor_inner(), temp_sensor_inner_device_state},
        {&Devices::temp_sensor_outer(), temp_sensor_outer_device_state}
    };

    bool is_hardware_setup = false;
    rwmutex_t hardware_state_lock;

    mutex_t dcdc_device_lock;
    mutex_t adcs_device_lock;
    mutex_t spike_and_hold_device_lock;
    mutex_t piksi_device_lock;
    mutex_t quake_device_lock;
    mutex_t gomspace_device_lock;
    mutex_t pressure_sensor_device_lock;
    mutex_t temp_sensor_inner_device_lock;
    mutex_t temp_sensor_outer_device_lock;
    mutex_t docking_motor_device_lock;
    mutex_t docking_switch_device_lock;

    bool check_is_functional(Devices::Device* d) {
        bool functional = d->is_functional();
        rwMtxWLock(&hardware_state_lock);
            hat.at(d).is_functional = functional;
        rwMtxWUnlock(&hardware_state_lock);
        return functional;
    }

    bool is_functional(Devices::Device* d) {
        rwMtxRLock(&hardware_state_lock);
            bool functional = hat.at(d).is_functional;
        rwMtxRUnlock(&hardware_state_lock);
        return functional;
    }
}

namespace ADCS {
    static Hardware::DeviceState adcs_gyroscope_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_magnetometer_1_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_magnetometer_2_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_magnetorquer_x_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_magnetorquer_y_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_magnetorquer_z_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_motorpot_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_motor_x_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_motor_y_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_motor_z_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_adc_motor_x_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_adc_motor_y_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_adc_motor_z_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_ssa_adc_1_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_ssa_adc_2_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_ssa_adc_3_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_ssa_adc_4_state = {false, false, false, false, 1};
    static Hardware::DeviceState adcs_ssa_adc_5_state = {false, false, false, false, 1};
    std::map<std::string, Hardware::DeviceState> adcs_hat {
        {"gyroscope", adcs_gyroscope_state},
        {"magnetometer_1", adcs_magnetometer_1_state},
        {"magnetometer_2", adcs_magnetometer_2_state},
        {"magnetorquer_x", adcs_magnetorquer_x_state},
        {"magnetorquer_y", adcs_magnetorquer_y_state},
        {"magnetorquer_z", adcs_magnetorquer_z_state},
        {"motorpot", adcs_motorpot_state},
        {"motor_x", adcs_motor_x_state},
        {"motor_y", adcs_motor_y_state},
        {"motor_z", adcs_motor_z_state},
        {"adc_motor_x", adcs_adc_motor_x_state},
        {"adc_motor_y", adcs_adc_motor_y_state},
        {"adc_motor_z", adcs_adc_motor_z_state},
        {"ssa_adc_1", adcs_ssa_adc_1_state},
        {"ssa_adc_2", adcs_ssa_adc_2_state},
        {"ssa_adc_3", adcs_ssa_adc_3_state},
        {"ssa_adc_4", adcs_ssa_adc_4_state},
        {"ssa_adc_5", adcs_ssa_adc_5_state}
    };
    rwmutex_t adcs_hardware_state_lock;
}
}