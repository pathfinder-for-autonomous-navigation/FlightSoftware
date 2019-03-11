/** THIS IS AN AUTOGENERATED FILE **/
#ifndef STATE_HISTORY_HOLDER_HPP_
#define STATE_HISTORY_HOLDER_HPP_

#include <circular_buffer.hpp>
#include <array>

#include <rwmutex.hpp>

namespace StateHistory {
namespace Propulsion {
  extern circular_buffer<float,10> tank_pressure_history;
  extern circular_buffer<int,10> inner_tank_temperature_history;
  extern circular_buffer<int,10> outer_tank_temperature_history;
  extern rwmutex_t propulsion_history_state_lock;
}
namespace Piksi {
  extern circular_buffer<int,10> iar_history;
  extern circular_buffer<int,10> nsats_history;
  extern circular_buffer<std::array<double, 3>,10> recorded_position_history;
  extern circular_buffer<std::array<double, 3>,10> recorded_velocity_history;
  extern circular_buffer<std::array<double, 3>,10> recorded_position_other_history;
  extern circular_buffer<std::array<double, 3>,10> position_history;
  extern circular_buffer<std::array<double, 3>,10> velocity_history;
  extern circular_buffer<std::array<double, 3>,10> position_other_history;
  extern circular_buffer<std::array<double, 3>,10> velocity_other_history;
  extern rwmutex_t piksi_history_state_lock;
}
namespace ADCS {
  extern circular_buffer<std::array<float, 3>,10> gyro_history;
  extern circular_buffer<std::array<float, 3>,10> gyro_fast_history;
  extern circular_buffer<std::array<float, 4>,10> attitude_cmd_history;
  extern circular_buffer<std::array<float, 4>,10> attitude_history;
  extern circular_buffer<std::array<float, 4>,10> attitude_fast_history;
  extern circular_buffer<std::array<float, 3>,10> rate_history;
  extern circular_buffer<std::array<float, 3>,10> rate_fast_history;
  extern circular_buffer<std::array<float, 3>,10> spacecraft_L_history;
  extern circular_buffer<std::array<float, 3>,10> spacecraft_L_fast_history;
  extern circular_buffer<std::array<float, 3>,10> magnetometer_history;
  extern circular_buffer<std::array<float, 3>,10> rwa_ramp_cmd_history;
  extern circular_buffer<std::array<float, 3>,10> mtr_cmd_history;
  extern circular_buffer<std::array<float, 3>,10> ssa_vector_history;
  extern rwmutex_t adcs_history_state_lock;
}
}

#endif