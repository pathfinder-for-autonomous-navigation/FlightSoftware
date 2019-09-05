/**
 * @file device_declarations.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains initializations for device objects representing devices in
 * the satellite.
 */

#include "device_states.hpp"
#include <EEPROM.h>
#include <i2c_t3.h>
#include "EEPROMAddresses.hpp"
#include "state_holder.hpp"

namespace Devices {
Gomspace *gomspace;
SpikeAndHold *spike_and_hold;
SystemOutput *system_output;
Piksi *piksi;
DCDC *dcdc;
QLocate *quake;
ADCS *adcs_system;
PressureSensor *pressure_sensor;
TempSensor *temp_sensor_inner;
TempSensor *temp_sensor_outer;
DockingMotor *docking_motor;
DockingSwitch *docking_switch;
}  // namespace Devices

namespace State {
namespace Hardware {
std::map<Devices::Device *, Hardware::DeviceState> hat;

bool is_hardware_setup = false;
rwmutex_t hardware_state_lock;

mutex_t dcdc_device_lock;
mutex_t adcs_device_lock;
mutex_t spike_and_hold_device_lock;
mutex_t piksi_device_lock;
mutex_t system_output_device_lock;
mutex_t quake_device_lock;
mutex_t gomspace_device_lock;
mutex_t pressure_sensor_device_lock;
mutex_t temp_sensor_inner_device_lock;
mutex_t temp_sensor_outer_device_lock;
mutex_t docking_motor_device_lock;
mutex_t docking_switch_device_lock;

bool check_is_functional(Devices::Device *d) {
    bool functional = d->is_functional();
    rwMtxWLock(&hardware_state_lock);
    hat.at(d).is_functional = functional;
    rwMtxWUnlock(&hardware_state_lock);
    return functional;
}

bool is_functional(Devices::Device *d) {
    rwMtxRLock(&hardware_state_lock);
    bool functional = hat.at(d).is_functional;
    rwMtxRUnlock(&hardware_state_lock);
    return functional;
}
}  // namespace Hardware

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
std::map<std::string, Hardware::DeviceState> adcs_hat{{"gyroscope", adcs_gyroscope_state},
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
                                                      {"ssa_adc_5", adcs_ssa_adc_5_state}};
rwmutex_t adcs_hardware_state_lock;
}  // namespace ADCS
}  // namespace State