/** @file helpers.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Implements helper functions for data history collection.
 */

#include "helpers.hpp"

void DataCollection::add_to_avg(std::array<float, 3> &avg, const std::array<float, 3> &val,
                                unsigned int *num_measurements) {
    std::array<float, 3> temp;
    for (int i = 0; i < 3; i++) {
        temp[i] = avg[i];
        temp[i] *= *num_measurements;
    }
    chSysLockFromISR();
    temp = val;
    chSysUnlockFromISR();
    (*num_measurements)++;
    for (int i = 0; i < 3; i++) {
        temp[i] /= *num_measurements;
        avg[i] = temp[i];
    }
}
