/** @file adcs_data.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains timers that periodically collect ADCS data and store it into a state buffer.
 */

#include <AttitudeEstimator.hpp>
#include "../state/state_holder.hpp"
#include "../state/state_history_holder.hpp"
#include <static_buffers.hpp>
#include "helpers.hpp"
#include "data_collection.hpp"

void DataCollection::initialize_adcs_history_timers() {
    // TODO
}