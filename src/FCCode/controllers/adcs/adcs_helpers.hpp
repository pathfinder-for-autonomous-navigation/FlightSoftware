#ifndef ADCS_POINTING_CALCULATORS_HPP_
#define ADCS_POINTING_CALCULATORS_HPP_

#include <array>

namespace ADCSControllers {
    void read_adcs_data();
    void get_command_attitude_in_eci(std::array<float, 4>* cmd_attitude_eci);
    void point_for_standby();
    void point_for_close_approach();
}

#endif