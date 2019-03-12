#include "constants.hpp"

// NOTE: Access to constants is controlled by the corresponding state lock.
namespace Constants {
    static unsigned int DUMMY_CONSTANT = 0;
    std::vector<unsigned int*> changeable_constants_map {
        &DUMMY_CONSTANT, // Allows for value to not get changed
        &Constants::Master::DOCKING_TIMEOUT,
        &Constants::Master::SAFE_HOLD_TIMEOUT,
        &Constants::Propulsion::VALVE_VENT_TIME,
        &Constants::Propulsion::VALVE_WAIT_TIME,
        &Constants::Propulsion::THRUSTER_PREPARATION_TIME,
        &Constants::Propulsion::STOP_PRESSURIZATION_TIME_DELTA,
        &Constants::Propulsion::WAIT_BETWEEN_PRESSURIZATIONS,
        &Constants::Quake::NUM_RETRIES,
        &Constants::Quake::UPLINK_TIMEOUT,
        &Constants::Quake::QUAKE_WAIT_PERIOD,
        &Constants::Quake::WAIT_BETWEEN_RETRIES,
    };

    namespace Master {
        #ifdef DEBUG
            unsigned int SAFE_HOLD_TIMEOUT = 3; // In seconds
            unsigned int DOCKING_TIMEOUT = 5;   // In seconds
        #else
            unsigned int SAFE_HOLD_TIMEOUT = 60*60*24; // 1 day, in seconds
            unsigned int DOCKING_TIMEOUT = 30*60; // 30 minutes, in seconds
        #endif
    }

    namespace Propulsion {
        unsigned int VALVE_VENT_TIME = 200;
        unsigned int VALVE_WAIT_TIME = 1000;
        unsigned int THRUSTER_PREPARATION_TIME = 300;
        unsigned int STOP_PRESSURIZATION_TIME_DELTA = 2000; 
        unsigned int WAIT_BETWEEN_PRESSURIZATIONS = 30000; // TODO
    }

    namespace Quake {
        unsigned int NUM_RETRIES = 5;
        #ifdef DEBUG
        unsigned int UPLINK_TIMEOUT = 10;
        unsigned int QUAKE_WAIT_PERIOD = 5;
        unsigned int WAIT_BETWEEN_RETRIES = 500;
        #else
        unsigned int UPLINK_TIMEOUT = 24*60*60;
        unsigned int QUAKE_WAIT_PERIOD = 5*60*50;
        unsigned int WAIT_BETWEEN_RETRIES = 2000;
        #endif
    }
}