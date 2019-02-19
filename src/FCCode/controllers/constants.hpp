/** @file constants.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains miscellaneous constants that the controller processes in this folder use.
 */

#ifndef CONSTANTS_HPP_
#define CONSTANTS_HPP_

namespace Constants {
    namespace Master {
        #ifdef DEBUG
            //! Defines how long the master controller safehold callback will wait prior to automatically exiting safe hold.
            constexpr unsigned int SAFE_HOLD_TIMEOUT = 3; // In seconds
            //! Seconds before initialization hold stops trying to detumble and instead tries to send a Quake packet while still being a spinny boi
            constexpr unsigned int INITIALIZATION_HOLD_DETUMBLE_WAIT = 5;
        #else
            //! Defines how long the master controller safehold callback will wait prior to automatically exiting safe hold.
            constexpr unsigned int SAFE_HOLD_TIMEOUT = 60*60*24; // 1 day, in seconds
            //! Seconds before initialization hold stops trying to detumble and instead tries to send a Quake packet while still being a spinny boi
            constexpr unsigned int INITIALIZATION_HOLD_DETUMBLE_WAIT = 30; 
        #endif
    }

    namespace Propulsion {
        //! Milliseconds that a valve is opened in order to pressurize or vent a tank.
        constexpr unsigned int VALVE_VENT_TIME = 500;
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
    }
}

#endif