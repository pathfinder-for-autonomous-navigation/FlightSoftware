#include "constants.hpp"

// NOTE: Access to constants is controlled by the corresponding state lock.
namespace Constants {
    static unsigned int DUMMY_CONSTANT = 0;
    std::vector<unsigned int*> changeable_constants_map {
        &DUMMY_CONSTANT, // Allows for value to not get changed
        &Constants::Master::DOCKING_RANGE,
        &Constants::Master::DOCKING_TIMEOUT,
        &Constants::Master::SAFE_HOLD_TIMEOUT,
        &Constants::Propulsion::VALVE_VENT_TIME,
        &Constants::Propulsion::VALVE_WAIT_TIME,
        &Constants::Propulsion::NUM_PRESSURIZATIONS,
        &Constants::Propulsion::WAIT_BETWEEN_PRESSURIZATIONS,
        &Constants::Propulsion::STOP_PRESSURIZATION_TIME_DELTA,
        &Constants::Piksi::CDGPS_RANGE,
        &Constants::Quake::NUM_RETRIES,
        &Constants::Quake::UPLINK_TIMEOUT,
        &Constants::Quake::QUAKE_WAIT_PERIOD,
        &Constants::Quake::WAIT_BETWEEN_RETRIES,
        &Constants::ADCS::ATTITUDE_CONTROLLER_KP,
        &Constants::ADCS::ATTITUDE_CONTROLLER_KD,
        &Constants::ADCS::MOMENTUM_CONTROLLER_K,
        &Constants::ADCS::GYROSCOPE_HEATER_KP,
        &Constants::ADCS::GYROSCOPE_HEATER_KI,
        &Constants::ADCS::GYROSCOPE_HEATER_KD,
    };
    rwmutex_t changeable_constants_lock;

    namespace ADCS {
        static float default_attitude_kp = 1; // TODO
        static float default_attitude_kd = 1; // TODO
        static float default_momentum_k = 1; // TODO
        static float default_heater_kp = 1; // TODO
        static float default_heater_ki = 1; // TODO
        static float default_heater_kd = 1; // TODO
        // Set default values by casting float to integer
        unsigned int ATTITUDE_CONTROLLER_KP = (unsigned int) default_attitude_kp;
        unsigned int ATTITUDE_CONTROLLER_KD = (unsigned int) default_attitude_kd; 
        unsigned int MOMENTUM_CONTROLLER_K = (unsigned int) default_momentum_k;
        unsigned int GYROSCOPE_HEATER_KP = (unsigned int) default_heater_kp;
        unsigned int GYROSCOPE_HEATER_KI = (unsigned int) default_heater_ki;
        unsigned int GYROSCOPE_HEATER_KD = (unsigned int) default_heater_kd;
    }

    namespace Master {
        unsigned int DOCKING_RANGE = 1; // In meters
        #ifdef DEBUG
            unsigned int SAFE_HOLD_TIMEOUT = 3; // In seconds
            unsigned int DOCKING_TIMEOUT = 5;   // In seconds
        #else
            unsigned int SAFE_HOLD_TIMEOUT = 60*60*24; // 1 day, in seconds
            unsigned int DOCKING_TIMEOUT = 30*60; // 30 minutes, in seconds
        #endif
    }

    namespace Piksi {
        unsigned int CDGPS_RANGE = 100;
    }

    namespace Propulsion {
        unsigned int VALVE_VENT_TIME = 200;
        unsigned int VALVE_WAIT_TIME = 1000;
        unsigned int NUM_PRESSURIZATIONS = 20;
        unsigned int WAIT_BETWEEN_PRESSURIZATIONS = 30000; // TODO
        unsigned int STOP_PRESSURIZATION_TIME_DELTA = 2000;
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