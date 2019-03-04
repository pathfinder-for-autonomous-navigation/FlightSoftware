/** @file state_history_holder.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains declarations for buffer objects that contain historical state data.
 */

#ifndef STATE_HISTORY_HOLDER_HPP_
#define STATE_HISTORY_HOLDER_HPP_

#include <array>
#include <circular_buffer.hpp>
#include <rwmutex.hpp>
#include "../data_collection/data_collection_rates.hpp"

namespace StateHistory {
  namespace ADCS {
    //! History of ADCS attitude commands
    extern circular_buffer<std::array<float, 4>, DataCollectionRates::ADCS::ATTITUDE_CMD> attitude_cmd_history;
    //! History of ADCS attitude
    extern circular_buffer<std::array<float, 4>, DataCollectionRates::ADCS::ATTITUDE> attitude_history;
    //! History of quick-timestep ADCS attitude
    extern circular_buffer<std::array<float, 4>, DataCollectionRates::ADCS::ATTITUDE_FAST> attitude_fast_history;
    //! History of ADCS angular rate
    extern circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::RATE> rate_history;
    //! History of quick-timestep ADCS angular rate
    extern circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::RATE_FAST> rate_fast_history;
    //! History of spacecraft angular momentum
    extern circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::L> spacecraft_L_history;
    //! History of quick-timestep spacecraft angular momentum
    extern circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::L_FAST> spacecraft_L_fast_history;
    //! History of gyroscope data
    extern circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::GYRO> gyro_history;
    //! History of quick-timestep gyroscope data
    extern circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::GYRO_FAST> gyro_fast_history;
    //! History of magnetometer data
    extern circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::MAGNETOMETER> magnetometer_history;
    //! History of reaction wheel ramp commands
    extern circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::RWA_RAMP_CMD> rwa_ramp_cmd_history;
    //! History of magnetorquer commands
    extern circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::MTR_CMD> mtr_cmd_history;
    //! History of sun sensor vector data
    extern circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::SSA_VECTOR> ssa_vector_history;
    //! Readers-writers lock that prevents multi-process modification of ADCS state history data.
    extern rwmutex_t adcs_state_history_lock;
  }

  namespace Propulsion {
    //! History of tank pressure
    extern circular_buffer<float, DataCollectionRates::Propulsion::TANK_PRESSURE> tank_pressure_history;
    //! History of inner tank temperature
    extern circular_buffer<float, DataCollectionRates::Propulsion::TANK_TEMPERATURE> inner_tank_temperature_history;
    //! History of outer tank temperature
    extern circular_buffer<float, DataCollectionRates::Propulsion::TANK_TEMPERATURE> outer_tank_temperature_history;
    //! Readers-writers lock that prevents multi-process modification of propulsion state history data.
    extern rwmutex_t propulsion_state_history_lock;
  }

  namespace GNC {
    //! History of (propagated) GPS position
    extern circular_buffer<std::array<double, 3>, DataCollectionRates::Piksi::POSITION> position_history;
    //! History of (propagated) GPS velocity
    extern circular_buffer<std::array<double, 3>, DataCollectionRates::Piksi::VELOCITY> velocity_history;
    //! History of (propagated) other satellite's GPS position
    extern circular_buffer<std::array<double, 3>, DataCollectionRates::Piksi::POSITION> position_other_history;
    //! History of (propagated) other satellite's recorded GPS velocity
    extern circular_buffer<std::array<double, 3>, DataCollectionRates::Piksi::VELOCITY> velocity_other_history;
    //! Readers-writers lock that prevents multi-process modification of GNC state history data.
    extern rwmutex_t gnc_state_history_lock;
  }

  namespace Piksi {
    //! History of recorded GPS position
    extern circular_buffer<std::array<double, 3>, DataCollectionRates::Piksi::POSITION> recorded_position_history;
    //! History of recorded GPS velocity
    extern circular_buffer<std::array<double, 3>, DataCollectionRates::Piksi::VELOCITY> recorded_velocity_history;
    //! History of other satellite's recorded GPS position
    extern circular_buffer<std::array<double, 3>, DataCollectionRates::Piksi::POSITION> recorded_position_other_history;
    //! History of other satellite's recorded GPS velocity
    extern circular_buffer<std::array<double, 3>, DataCollectionRates::Piksi::VELOCITY> recorded_velocity_other_history;
    //! Readers-writers lock that prevents multi-process modification of Piksi state history data.
    extern rwmutex_t piksi_state_history_lock;
  }
}

#endif