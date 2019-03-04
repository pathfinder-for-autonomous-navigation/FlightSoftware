/** @file state_history_holder.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains initializations for buffer objects that contain historical state data.
 */

#include "state_history_holder.hpp"

namespace StateHistory {
  namespace ADCS {
    circular_buffer<std::array<float, 4>, DataCollectionRates::ADCS::ATTITUDE_CMD> attitude_cmd_history;
    circular_buffer<std::array<float, 4>, DataCollectionRates::ADCS::ATTITUDE> attitude_history;
    circular_buffer<std::array<float, 4>, DataCollectionRates::ADCS::ATTITUDE_FAST> attitude_fast_history;
    circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::RATE> rate_history;
    circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::RATE_FAST> rate_fast_history;
    circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::L> spacecraft_L_history;
    circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::L_FAST> spacecraft_L_fast_history;
    circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::GYRO> gyro_history;
    circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::GYRO_FAST> gyro_fast_history;
    circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::MAGNETOMETER> magnetometer_history;
    circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::RWA_RAMP_CMD> rwa_ramp_cmd_history;
    circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::MTR_CMD> mtr_cmd_history;
    circular_buffer<std::array<float, 3>, DataCollectionRates::ADCS::SSA_VECTOR> ssa_vector_history;
    rwmutex_t adcs_state_history_lock;
  }

  namespace Propulsion {
    circular_buffer<float, DataCollectionRates::Propulsion::TANK_PRESSURE> tank_pressure_history;
    circular_buffer<float, DataCollectionRates::Propulsion::TANK_TEMPERATURE> inner_tank_temperature_history;
    circular_buffer<float, DataCollectionRates::Propulsion::TANK_TEMPERATURE> outer_tank_temperature_history;
    rwmutex_t propulsion_state_history_lock;
  }

  namespace GNC {
    circular_buffer<std::array<double, 3>, DataCollectionRates::Piksi::POSITION> position_history;
    circular_buffer<std::array<double, 3>, DataCollectionRates::Piksi::VELOCITY> velocity_history;
    circular_buffer<std::array<double, 3>, DataCollectionRates::Piksi::POSITION> position_other_history;
    circular_buffer<std::array<double, 3>, DataCollectionRates::Piksi::VELOCITY> velocity_other_history;
    rwmutex_t gnc_state_history_lock;
  }

  namespace Piksi {
    circular_buffer<std::array<double, 3>, DataCollectionRates::Piksi::POSITION> recorded_position_history;
    circular_buffer<std::array<double, 3>, DataCollectionRates::Piksi::VELOCITY> recorded_velocity_history;
    circular_buffer<std::array<double, 3>, DataCollectionRates::Piksi::POSITION> recorded_position_other_history;
    circular_buffer<std::array<double, 3>, DataCollectionRates::Piksi::VELOCITY> recorded_velocity_other_history;
    rwmutex_t piksi_state_history_lock;
  }
}