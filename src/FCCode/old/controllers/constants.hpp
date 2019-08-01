/** @file constants.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains miscellaneous constants that the controller processes in this
 * folder use.
 */

#ifndef CONSTANTS_HPP_
#define CONSTANTS_HPP_

#include <map>
#include <tensor.hpp>
#include <vector>
#include "../state/device_states.hpp"
#include "../state/state_holder.hpp"

// NOTE: Access to constants is controlled by the corresponding state lock.
namespace Constants {
//! Map pointing to changeable constants
extern std::vector<unsigned int *> changeable_constants_map;
extern rwmutex_t changeable_constants_lock;
//! Helper function to read constant value from the above table
inline unsigned int read(unsigned int &val) { return State::read(val, changeable_constants_lock); }

namespace Master {
extern unsigned int DOCKING_RANGE;  // In meters
#ifdef DEBUG_ENABLED
//! Defines how long the master controller safehold callback
//! will wait prior to automatically exiting safe hold.
extern unsigned int SAFE_HOLD_TIMEOUT;
//! Seconds before initialization hold stops trying to detumble and instead
//! tries to send a Quake packet while still spinning
constexpr unsigned int INITIALIZATION_HOLD_DETUMBLE_WAIT = 5;
//! Seconds before docking mode is automatically exited and standby mode is
//! triggered
extern unsigned int DOCKING_TIMEOUT;
#else
//! Defines how long the master controller safehold callback
//! will wait prior to automatically exiting safe hold.
extern unsigned int SAFE_HOLD_TIMEOUT;
//! Seconds before initialization hold stops trying to detumble and instead
//! tries to send a Quake packet while still spinning
constexpr unsigned int INITIALIZATION_HOLD_DETUMBLE_WAIT = 30;
//! Seconds before docking mode is automatically exited and standby mode is
//! triggered
extern unsigned int DOCKING_TIMEOUT;
#endif
constexpr unsigned int ORBIT_PERIOD_MS =
    2 * 60 * 60 * 1000;  // Approximate orbital period in milliseconds--assuming 2-hour orbit
constexpr float SPACECRAFT_MASS = 1.0;  // TODO fix
}  // namespace Master

namespace ADCS {
//! Maximum angular rate magnitude that is considered "stable".
constexpr float MAX_STABLE_ANGULAR_RATE = 0.05f;  // TODO set value
//! Maximum possible angular rate magnitude of spacecraft. (rad/s)
constexpr float MAX_ANGULAR_RATE = 2.2;
//! Maximum possible gyroscope reading along one axis. (rad/s)
constexpr float MIN_GYRO_VALUE = -2.2;
//! Maximum possible gyroscope reading along one axis. (rad/s)
constexpr float MAX_GYRO_VALUE = 2.2;
//! Maximum possible magnetic field vector reading magnitude. (T)
constexpr float MAX_MAGNETOMETER_READING = 0.005;
//! Maximum possible ramp command magnitude. (rad/s^2)
constexpr float MAX_RAMP_CMD = 310.2;
//! Minimum possible magnetotorquer command along one axis. (A m^2)
constexpr float MIN_MTR_CMD = 0.008;  // TODO
//! Maximum possible magnetotorquer command along one axis. (A m^2)
constexpr float MAX_MTR_CMD = 0.008;  // TODO
//! Minimum possible voltage reading of one sun sensor. (V)
constexpr float MIN_SUN_SENSOR_VALUE = 0;  // TODO
//! Maximum possible voltage reading of one sun sensor. (V)
constexpr float MAX_SUN_SENSOR_VALUE = 5;  // TODO

//! Attitude controller proportional gain--"raw" integral value updated by
//! uplink
extern unsigned int ATTITUDE_CONTROLLER_KP;
//! Attitude controller derivative gain--"raw" integral value updated by uplink
extern unsigned int ATTITUDE_CONTROLLER_KD;
//! Momentum controller gain--"raw" integral value updated by uplink
extern unsigned int MOMENTUM_CONTROLLER_K;
//! Gyroscope heater proportional gain--"raw" integral value updated by uplink
extern unsigned int GYROSCOPE_HEATER_KP;
//! Gyroscope heater integral gain--"raw" integral value updated by uplink
extern unsigned int GYROSCOPE_HEATER_KI;
//! Gyroscope heater derivative gain--"raw" integral value updated by uplink
extern unsigned int GYROSCOPE_HEATER_KD;
}  // namespace ADCS

namespace Gomspace {
struct limit_t {
    short min;
    short max;
};
constexpr limit_t boost_voltage_limits = {0, 0};               // TODO
constexpr limit_t temperature_limits = {};                     // TODO
constexpr limit_t quake_limits = {0, 0};                       // TODO
constexpr limit_t adcs_system_limits = {0, 0};                 // TODO
constexpr limit_t spike_and_hold_limits = {0, 0};              // TODO
constexpr limit_t piksi_limits = {0, 0};                       // TODO
constexpr limit_t individual_boost_converter_limits = {0, 0};  // TODO
constexpr limit_t total_boost_converter_limits = {0, 0};       // TODO
constexpr limit_t battery_current_limits = {0, 0};             // TODO
constexpr unsigned int SAFE_VOLTAGE = Devices::Gomspace::FC_NORMAL;
}  // namespace Gomspace

namespace Piksi {
//! Minimum range in which we expect a RTK lock, in meters
extern unsigned int CDGPS_RANGE;
}  // namespace Piksi

namespace Propulsion {
//! Milliseconds that a valve is opened in order to pressurize or vent a tank.
extern unsigned int VALVE_VENT_TIME;
//! Milliseconds that we wait between opening the same valve.
extern unsigned int VALVE_WAIT_TIME;
//! Milliseconds before the actual thruster firing that the tank 2
//! pressure-controlling loop is stopped.
extern unsigned int STOP_PRESSURIZATION_TIME_DELTA;
//! Maximum allowable temperature of inner tank
extern unsigned int MAX_INNER_TANK_TEMPERATURE;
//! Maximum allowable temperature of outer tank
extern unsigned int MAX_OUTER_TANK_TEMPERATURE;
//! Maximum allowable pressure of outer tank
extern unsigned int MAX_OUTER_TANK_PRESSURE;
//! Amount of time, in milliseconds, to wait between intertank ventings
extern unsigned int WAIT_BETWEEN_PRESSURIZATIONS;
//! Number of firings to use to pressurize tank
extern unsigned int NUM_PRESSURIZATIONS;
//! Maximum amount of time between end of pressurization and actual firing
extern unsigned int STOP_PRESSURIZATION_TIME_DELTA;
//! Helper function to compute amount of time required to pressurize tank
inline unsigned int thruster_preparation_time() {
    unsigned int num_pressurizations = Constants::read(Constants::Propulsion::NUM_PRESSURIZATIONS);
    unsigned int wait_between_pressurizations =
        Constants::read(Constants::Propulsion::WAIT_BETWEEN_PRESSURIZATIONS);
    return num_pressurizations * wait_between_pressurizations;
}
//! Required outer tank pressure prior to initiating a firing
constexpr float PRE_FIRING_OUTER_TANK_PRESSURE = 0;  // TODO
//! Maximum allowable impulse magnitude for a particular firing, in kg m/s
constexpr float MAX_FIRING_IMPULSE = 1.0;  // TODO
//! Vector directions, relative to body frame, in which the nozzles point.
// Maps each vector to the logical valve # in the Spike and Hold driver.
extern std::map<unsigned char, const pla::Vec3f> NOZZLE_VECTORS;
}  // namespace Propulsion

namespace Quake {
//! Number of times to try sending a packet
extern unsigned int NUM_RETRIES;
#ifdef DEBUG_ENABLED
//! Maximum number of seconds before safe hold happens because of an uplink
//! timeout
extern unsigned int UPLINK_TIMEOUT;
//! Period to wait prior to forcibly trying a downlink (seconds)
extern unsigned int QUAKE_WAIT_PERIOD;
//! Interval between successive downlink retries (milliseconds)
extern unsigned int WAIT_BETWEEN_RETRIES;
#else
//! Maximum number of seconds before safe hold happens because of an uplink
//! timeout
extern unsigned int UPLINK_TIMEOUT;
//! Period to wait prior to forcibly trying a downlink (seconds)
extern unsigned int QUAKE_WAIT_PERIOD;
//! Interval between successive downlink retries (milliseconds)
extern unsigned int WAIT_BETWEEN_RETRIES;
#endif
}  // namespace Quake
}  // namespace Constants

#endif