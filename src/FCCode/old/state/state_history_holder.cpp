/** THIS IS AN AUTOGENERATED FILE **/
#include "state_history_holder.hpp"

namespace StateHistory {
namespace Propulsion {
circular_buffer<unsigned int, 10> tank_pressure_history;
circular_buffer<int, 10> inner_tank_temperature_history;
circular_buffer<int, 10> outer_tank_temperature_history;

rwmutex_t propulsion_history_state_lock;
}  // namespace Propulsion
namespace Piksi {
circular_buffer<unsigned int, 10> iar_history;
circular_buffer<unsigned int, 10> nsats_history;
circular_buffer<unsigned int, 10> is_float_rtk_history;
circular_buffer<unsigned int, 10> is_fixed_rtk_history;
circular_buffer<std::array<double, 3>, 10> recorded_position_history;
circular_buffer<std::array<double, 3>, 10> recorded_velocity_history;
circular_buffer<std::array<double, 3>, 10> recorded_position_other_history;
circular_buffer<std::array<double, 3>, 10> position_history;
circular_buffer<std::array<double, 3>, 10> velocity_history;
circular_buffer<std::array<double, 3>, 10> position_other_history;
circular_buffer<std::array<double, 3>, 10> velocity_other_history;

rwmutex_t piksi_history_state_lock;
}  // namespace Piksi
namespace ADCS {
circular_buffer<std::array<float, 3>, 10> gyro_history;
circular_buffer<std::array<float, 3>, 10> gyro_fast_history;
circular_buffer<std::array<float, 4>, 10> attitude_cmd_history;
circular_buffer<std::array<float, 4>, 10> attitude_history;
circular_buffer<std::array<float, 4>, 10> attitude_fast_history;
circular_buffer<std::array<float, 3>, 10> rate_history;
circular_buffer<std::array<float, 3>, 10> rate_fast_history;
circular_buffer<std::array<float, 3>, 10> spacecraft_L_history;
circular_buffer<std::array<float, 3>, 10> spacecraft_L_fast_history;
circular_buffer<std::array<float, 3>, 10> magnetometer_history;
circular_buffer<std::array<float, 3>, 10> rwa_ramp_cmd_history;
circular_buffer<std::array<float, 3>, 10> mtr_cmd_history;
circular_buffer<std::array<float, 3>, 10> ssa_vector_history;

rwmutex_t adcs_history_state_lock;
}  // namespace ADCS
}  // namespace StateHistory