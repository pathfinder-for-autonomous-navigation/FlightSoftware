/** @file data_collection_rates.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Specifies rates at which data is being collected and inserted into history buffers.
 */

#ifndef DATA_COLLECTION_RATES_HPP_
#define DATA_COLLECTION_RATES_HPP_

/**
 * @addtogroup comms_utils
 */
/** @{ **/

/**
 * @namespace DataCollectionRates Specifies the millisecond intervals in which data should be collected
 * for each data point whose history is tracked.
 * 
 * In addition to the intervals, this namespace also helpfully tracks the number of times each data point
 * is collected over the course of the period between data downlinks. This reduces code bloat in the circular_buffer
 * declarations of state_history.cpp
 * 
 * All times are in milliseconds.
 */
namespace DataCollectionRates {
    //! Five minutes, in milliseconds.
    constexpr unsigned int FIVE_MINUTES = 300000;
    //! Ten seconds, in milliseconds. We need this field because some data points are collected very quickly out of the last ten seconds. 
    constexpr unsigned int TEN_SECONDS = 10000;
    //! Two seconds, in milliseconds. We need this field because some data points are collected very quickly out of the last two seconds.
    constexpr unsigned int TWO_SECONDS = 2000;
    
    /** @namespace ADCS ADCS-specific data collection rate constants. **/
    namespace ADCS {
        //! Interval between attitude command recordings.
        constexpr unsigned int ATTITUDE_CMD_INTERVAL  = 30000;
        //! Interval between real attitude recordings.
        constexpr unsigned int ATTITUDE_INTERVAL      = 30000;
        //! Interval between quick-timestep attitude recordings.
        constexpr unsigned int ATTITUDE_FAST_INTERVAL = 1000;
        //! Interval between attitude averagings (over the slow timestep).
        constexpr unsigned int ATTITUDE_AVG_INTERVAL  = 100;
        //! Interval between real angular rate recordings.
        constexpr unsigned int RATE_INTERVAL          = 30000;
         //! Interval between quick-timestep angular rate recordings.
        constexpr unsigned int RATE_FAST_INTERVAL     = 1000;
        //! Interval between angular rate averagings (over the slow timestep).
        constexpr unsigned int RATE_AVG_INTERVAL      = 100;
        //! Interval between spacecraft angular momentum recordings. This angular momentum
        // computation is based on reaction wheel speed only, and is computed in the
        // body frame of the spacecraft.
        constexpr unsigned int L_INTERVAL             = 30000;
        //! Interval between quick-timestep spacecraft angular momentum recordings.
        constexpr unsigned int L_FAST_INTERVAL        = 1000;
        //! Interval between spacecraft angular momentum averagings (over the slow timestep).
        constexpr unsigned int L_AVG_INTERVAL         = 100;
        //! Interval between gyroscope data recordings.
        constexpr unsigned int GYRO_INTERVAL          = 20000;
        //! Interval between quick-timestep gyroscope data recordings.
        constexpr unsigned int GYRO_FAST_INTERVAL     = 100;
        //! Interval between gyroscope data averagings (over the slow timestep).
        constexpr unsigned int GYRO_AVG_INTERVAL      = 100;
        //! Interval between magnetometer data recordings.
        constexpr unsigned int MAGNETOMETER_INTERVAL  = 15000;
        //! Interval between magnetometer data averagings.
        constexpr unsigned int MAGNETOMETER_AVG_INTERVAL = 100;
        //! Interval between reaction wheel ramp command recordings.
        constexpr unsigned int RWA_RAMP_CMD_INTERVAL  = 30000;
        //! Interval between magnetorquer command recordings.
        constexpr unsigned int MTR_CMD_INTERVAL       = 30000;
        //! Interval between sun sensor vector recordings.
        constexpr unsigned int SSA_VECTOR_INTERVAL    = 30000;

        //! Number of times attitude command data is collected per downlink packet.
        constexpr unsigned int ATTITUDE_CMD  = FIVE_MINUTES / ATTITUDE_CMD_INTERVAL;
        //! Number of times attitude data is collected per downlink packet.
        constexpr unsigned int ATTITUDE      = FIVE_MINUTES / ATTITUDE_INTERVAL;
        //! Number of times quick-timestep attitude data is collected per downlink packet.
        constexpr unsigned int ATTITUDE_FAST = TEN_SECONDS  / ATTITUDE_FAST_INTERVAL;
        //! Number of times angular rate data is collected per downlink packet.
        constexpr unsigned int RATE          = FIVE_MINUTES / RATE_INTERVAL;
        //! Number of times quick-timestep angular rate data is collected per downlink packet.
        constexpr unsigned int RATE_FAST     = TEN_SECONDS  / RATE_FAST_INTERVAL;
        //! Number of times spacecraft angular momentum data is collected per downlink packet.
        constexpr unsigned int L             = FIVE_MINUTES / L_INTERVAL;
        //! Number of times quick-timestep spacecraft angular momentum data is collected per downlink packet.
        constexpr unsigned int L_FAST        = TEN_SECONDS  / L_FAST_INTERVAL;
        //! Number of times gyroscope data is collected per downlink packet.
        constexpr unsigned int GYRO          = FIVE_MINUTES / GYRO_INTERVAL;
        //! Number of times quick-timestep gyroscope data is collected per downlink packet.
        constexpr unsigned int GYRO_FAST     = TWO_SECONDS  / GYRO_FAST_INTERVAL;
        //! Number of times magnetometer data is collected per downlink packet.
        constexpr unsigned int MAGNETOMETER  = FIVE_MINUTES / MAGNETOMETER_INTERVAL;
        //! Number of times reaction wheel ramp command data is collected per downlink packet.
        constexpr unsigned int RWA_RAMP_CMD  = FIVE_MINUTES / RWA_RAMP_CMD_INTERVAL;
        //! Number of times magnetorquer command data is collected per downlink packet.
        constexpr unsigned int MTR_CMD       = FIVE_MINUTES / MTR_CMD_INTERVAL;
        //! Number of times sun sensor vector data is collected per downlink packet.
        constexpr unsigned int SSA_VECTOR    = FIVE_MINUTES / SSA_VECTOR_INTERVAL;
    }

    /** @namespace Piksi Piksi-specific data collection rate constants. **/
    namespace Piksi {
        //! Interval between GPS position recordings.
        constexpr unsigned int POSITION_INTERVAL = 60000;
        //! Interval between GPS velocity recordings.
        constexpr unsigned int VELOCITY_INTERVAL = 60000;

        //! Number of times GPS position data is collected per downlink packet.
        constexpr unsigned int POSITION = FIVE_MINUTES / POSITION_INTERVAL;
        //! Number of times GPS velocity data is collected per downlink packet.
        constexpr unsigned int VELOCITY = FIVE_MINUTES / VELOCITY_INTERVAL;
    }

    /** @namespace Propulsion Propulsion-specific data collection rate constants. **/
    namespace Propulsion {
        //! Interval between tank pressure recordings.
        constexpr unsigned int TANK_PRESSURE_INTERVAL = 10000;
        //! Interval between tank temperature recordings.
        constexpr unsigned int TANK_TEMPERATURE_INTERVAL = 60000;

        //! Number of times tank pressure data is collected per downlink packet.
        constexpr unsigned int TANK_PRESSURE    = FIVE_MINUTES / TANK_PRESSURE_INTERVAL;
        //! Number of times tank temperature data is collected per downlink packet.
        constexpr unsigned int TANK_TEMPERATURE = FIVE_MINUTES / TANK_TEMPERATURE_INTERVAL;
    }
}

/** @} **/

#endif