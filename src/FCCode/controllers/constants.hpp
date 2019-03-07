/** @file constants.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains miscellaneous constants that the controller processes in this folder use.
 */

#ifndef CONSTANTS_HPP_
#define CONSTANTS_HPP_

#include "../state/device_states.hpp"
#include <tensor.hpp>
#include <map>

namespace Constants {
    namespace Master {
        #ifdef DEBUG
            //! Defines how long the master controller safehold callback will wait prior to automatically exiting safe hold.
            constexpr unsigned int SAFE_HOLD_TIMEOUT = 3; // In seconds
            //! Seconds before initialization hold stops trying to detumble and instead tries to send a Quake packet while still being a spinny boi
            constexpr unsigned int INITIALIZATION_HOLD_DETUMBLE_WAIT = 5;
            //! Seconds before docking mode is automatically exited and standby mode is triggered
            constexpr unsigned int DOCKING_TIMEOUT = 5;
        #else
            //! Defines how long the master controller safehold callback will wait prior to automatically exiting safe hold.
            constexpr unsigned int SAFE_HOLD_TIMEOUT = 60*60*24; // 1 day, in seconds
            //! Seconds before initialization hold stops trying to detumble and instead tries to send a Quake packet while still being a spinny boi
            constexpr unsigned int INITIALIZATION_HOLD_DETUMBLE_WAIT = 30; 
            //! Seconds before docking mode is automatically exited and standby mode is triggered
            constexpr unsigned int DOCKING_TIMEOUT = 30*60; 
        #endif
        constexpr unsigned int ORBIT_PERIOD_MS = 2*60*60*1000; // Approximate orbital period in milliseconds--assuming 2-hour orbit
        constexpr float SPACECRAFT_MASS = 1.0; // TODO fix
    }

    namespace ADCS {
        //! Maximum angular rate magnitude that is considered "stable".
        constexpr float MAX_STABLE_ANGULAR_RATE = 0.0f; // TODO set value
        //! Maximum possible angular rate magnitude of spacecraft.
        constexpr float MAX_ANGULAR_RATE = 2.2;
        //! Maximum possible gyroscope reading along one axis.
        constexpr float MIN_GYRO_VALUE = -2.2;
        //! Maximum possible gyroscope reading along one axis.
        constexpr float MAX_GYRO_VALUE = 2.2;
        //! Maximum possible magnetic field vector reading magnitude.
        constexpr float MAX_MAGNETOMETER_READING = 0.005;
        //! Maximum possible ramp command magnitude.
        constexpr float MAX_RAMP_CMD = 310.2;
        //! Minimum possible magnetotorquer command along one axis.
        constexpr float MIN_MTR_CMD = 0; // TODO
        //! Maximum possible magnetotorquer command along one axis.
        constexpr float MAX_MTR_CMD = 0; // TODO
        //! Minimum possible reading of one sun sensor.
        constexpr float MIN_SUN_SENSOR_VALUE = 0; // TODO
        //! Maximum possible reading of one sun sensor.
        constexpr float MAX_SUN_SENSOR_VALUE = 0; // TODO
    }

    namespace Gomspace {
        struct limit_t {
            short min;
            short max;
        };
        constexpr limit_t boost_voltage_limits = {0,0}; // TODO
        constexpr limit_t temperature_limits = {}; // TODO
        constexpr unsigned int SAFE_VOLTAGE = Devices::Gomspace::FC_NORMAL;
    }

    namespace Propulsion {
        //! Milliseconds that a valve is opened in order to pressurize or vent a tank.
        constexpr unsigned int VALVE_VENT_TIME = 200;
        //! Milliseconds that we wait between opening the same valve.
        constexpr unsigned int VALVE_WAIT_TIME = 1000;
        //! Seconds before the actual thruster firing that a propellant-maximizing attitude adjustment is made.
        constexpr unsigned int THRUSTER_PREPARATION_TIME = 300; 
        //! Milliseconds before the actual thruster firing that the tank 2 pressure-controlling loop is stopped.
        constexpr unsigned int STOP_PRESSURIZATION_TIME_DELTA = 2000; 
        //! Maximum delta-v threshold for NOT requiring orientation adjustment for a propulsion manuever.
        constexpr unsigned short int REORIENTATION_DELTA_V_THRESHOLD = 0; // TODO fix.
        //! Minimum battery threshold for propulsion manuever orientation adjustment.
        constexpr unsigned short int REORIENTATION_BATT_THRESHOLD = 0; // TODO fix.
        //! Maximum allowable temperature of inner tank
        constexpr float MAX_INNER_TANK_TEMPERATURE = 48;
        //! Maximum allowable temperature of outer tank
        constexpr float MAX_OUTER_TANK_TEMPERATURE = 48;
        //! Maximum allowable pressure of outer tank
        constexpr float MAX_OUTER_TANK_PRESSURE = 100;
        //! Amount of time, in milliseconds, to wait between intertank ventings
        constexpr unsigned int WAIT_BETWEEN_PRESSURIZATIONS = 30000; // TODO
        //! Required outer tank pressure prior to initiating a firing
        constexpr float PRE_FIRING_OUTER_TANK_PRESSURE = 0; // TODO
        //! Maximum allowable impulse magnitude for a particular firing, in kg m/s
        constexpr float MAX_FIRING_IMPULSE = 1.0; // TODO
        //! Vector directions, relative to body frame, in which the nozzles point.
        // Maps each vector to the logical valve # in the Spike and Hold driver.
        extern std::map<unsigned char, const pla::Vec3f> NOZZLE_VECTORS;
    }

    namespace Quake {
        //! Number of times to try sending a packet
        constexpr unsigned int NUM_RETRIES = 5;
        #ifdef DEBUG
        //! Maximum number of seconds before safe hold happens because of an uplink timeout
        constexpr unsigned int UPLINK_TIMEOUT = 10;
        //! Period to wait prior to forcibly trying a downlink (seconds)
        constexpr unsigned int QUAKE_WAIT_PERIOD = 5;
        //! Interval between successive downlink retries (milliseconds)
        constexpr unsigned int WAIT_BETWEEN_RETRIES = 500;
        #else
        //! Maximum number of seconds before safe hold happens because of an uplink timeout
        constexpr unsigned int UPLINK_TIMEOUT = 24*60*60;
        //! Period to wait prior to forcibly trying a downlink (seconds)
        constexpr unsigned int QUAKE_WAIT_PERIOD = 5*60*50;
        //! Interval between successive downlink retries (milliseconds)
        constexpr unsigned int WAIT_BETWEEN_RETRIES = 2000;
        #endif
    }
}

#endif