/** @file device_declarations.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains definitions for device objects representing devices in the satellite.
 */

#ifndef DEVICE_DECLARATIONS_HPP_
#define DEVICE_DECLARATIONS_HPP_

#include <Device.hpp>
#include <rwmutex.hpp>
#include <map>
#include "state_definitions.hpp"
#include <Gomspace.hpp>
#include <Piksi.hpp>
#include <FakePiksi.hpp>
#include <SystemOutput.hpp>
#include <SpikeAndHold.hpp>
#include <QLocate.hpp>
#include <ADCS.hpp>
#include <PressureSensor.hpp>
#include <TempSensor.hpp>
#include <DCDC.hpp>
#include <DockingMotor.hpp>
#include <DockingSwitch.hpp>

namespace Devices {
    //! Gomspace logical device used by flight controller code
    Gomspace& gomspace();
    //! Piksi logical device used by flight controller code
    Piksi& piksi();
    //! Debug device for system internals
    SystemOutput& system_output();
    //! Spike and Hold logical device used by flight controller code
    SpikeAndHold& spike_and_hold();
    //! DCDC logical device used by flight controller code
    DCDC& dcdc();
    //! Quake logical device used by flight controller code
    QLocate& quake();
    //! ADCS logical device used by flight controller code
    ADCS& adcs_system();
    //! Main pressure sensor in outer tank
    PressureSensor& pressure_sensor();
    //! Temperature sensor in inner tank
    TempSensor& temp_sensor_inner();
    //! Temperature sensor in outer tank
    TempSensor& temp_sensor_outer();
    //! Docking motor
    DockingMotor& docking_motor();
    //! Docking switch
    DockingSwitch& docking_switch();
}

namespace State {
namespace Hardware {
    //! Readers-writers lock that prevents multi-process modification of hardware availability table data.
    extern rwmutex_t hardware_state_lock;
    //! Maps device names to their state. This is an ordered map, in order to guarantee
    // that the devices are always listed in the same order.
    extern std::map<Devices::Device*, DeviceState> hat;
    //! Updates state of HAVT table by actually polling the device
    bool check_is_functional(Devices::Device* d);
    //! Returns state of device stored in HAVT table
    bool is_functional(Devices::Device* d);
    //! Maps devices to the corresponding power output.
    extern std::map<Devices::Device*, unsigned char> power_outputs;
    //! Readers-writers lock that prevents multi-process modification of hardware availability table data.
    extern bool is_hardware_setup;

    //! Protects access to DCDC enabling/disabling.
    extern mutex_t dcdc_device_lock;
    //! Protects access to ADCS device. May be used by master process to rewrite device defaults.
    extern mutex_t adcs_device_lock;
    //! Protects access to Gomspace device. May be used by master process to rewrite device defaults.
    extern mutex_t gomspace_device_lock;
    //! Protects access to propulsion actuation.
    extern mutex_t spike_and_hold_device_lock;
    //! Protects access to Piksi device. May be used by master process to rewrite device defaults.
    extern mutex_t piksi_device_lock;
    //! Protects access to System Output device (used during development/debugging).
    extern mutex_t system_output_device_lock;
    //! Protects access to Quake device. May be used by master process to rewrite device defaults.
    extern mutex_t quake_device_lock;
    //! Protects access to device_pressure sensor.
    extern mutex_t pressure_sensor_device_lock;
    //! Protects access to inner tank temperature sensor.
    extern mutex_t temp_sensor_inner_device_lock;
    //! Protects access to outer tank temperature sensor.
    extern mutex_t temp_sensor_outer_device_lock;
    //! Protects access to docking motor.
    extern mutex_t docking_motor_device_lock;
    //! Protects access to docking switch.
    extern mutex_t docking_switch_device_lock;
}

namespace ADCS {
    //! Hardware availability table of devices attached to ADCS.
    extern std::map<std::string, Hardware::DeviceState> adcs_hat;
    //! Readers-writers lock that prevents multi-process modification of hardware availability table data.
    extern rwmutex_t adcs_hardware_state_lock;
}
}

#endif