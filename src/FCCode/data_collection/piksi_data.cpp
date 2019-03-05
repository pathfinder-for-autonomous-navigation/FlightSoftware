/** @file piksi_data.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains timers that periodically collect Piksi GPS data and store it into a history buffer.
 */

#include "../state/state_holder.hpp"
#include "../state/state_history_holder.hpp"
#include "helpers.hpp"
#include "data_collection.hpp"
#include "data_collection_rates.hpp"

static void save_position_history(void* arg) {
    DataCollection::add_to_buffer<std::array<double,3>, DataCollectionRates::Piksi::POSITION>(
        StateHistory::Piksi::recorded_position_history,
        State::Piksi::recorded_gps_position);
}

static void save_velocity_history(void* arg) {
    DataCollection::add_to_buffer<std::array<double,3>, DataCollectionRates::Piksi::VELOCITY>(
        StateHistory::Piksi::recorded_velocity_history,
        State::Piksi::recorded_gps_velocity);
}

static void save_position_other_history(void* arg) {
    DataCollection::add_to_buffer<std::array<double,3>, DataCollectionRates::Piksi::POSITION>(
        StateHistory::Piksi::recorded_position_other_history,
        State::Piksi::recorded_gps_position_other);
}

static virtual_timer_t position_history_timer;
static virtual_timer_t velocity_history_timer;
static virtual_timer_t position_other_history_timer;

void DataCollection::initialize_piksi_history_timers() {
    chVTObjectInit(&position_history_timer);
    chVTObjectInit(&velocity_history_timer);
    chVTObjectInit(&position_other_history_timer);

    rwMtxObjectInit(&StateHistory::Piksi::piksi_state_history_lock);

    chSysLock();
        chVTSet(&position_history_timer, DataCollectionRates::Piksi::POSITION_INTERVAL, save_position_history, NULL);
        chVTSet(&velocity_history_timer, DataCollectionRates::Piksi::VELOCITY_INTERVAL, save_velocity_history, NULL);
        chVTSet(&position_other_history_timer, DataCollectionRates::Piksi::VELOCITY_INTERVAL, save_position_other_history, NULL);
    chSysUnlock();
}