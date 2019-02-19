/** @file propulsion_data.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains timers that periodically collect propulsion data and store it into a history buffer.
 */

#include "../state/state_holder.hpp"
#include "../state/state_history_holder.hpp"
#include "helpers.hpp"
#include "data_collection.hpp"
#include "data_collection_rates.hpp"

static void save_inner_tank_temperature_history(void* arg) {
    DataCollection::add_to_buffer<float, DataCollectionRates::Propulsion::TANK_TEMPERATURE>(
        StateHistory::Propulsion::inner_tank_temperature_history,
        State::Propulsion::tank_inner_temperature);
}

static void save_outer_tank_temperature_history(void* arg) {
    DataCollection::add_to_buffer<float, DataCollectionRates::Propulsion::TANK_TEMPERATURE>(
        StateHistory::Propulsion::outer_tank_temperature_history, 
        State::Propulsion::tank_outer_temperature);
}

static void save_pressure_history(void* arg) {
    DataCollection::add_to_buffer<float, DataCollectionRates::Propulsion::TANK_PRESSURE>(
        StateHistory::Propulsion::tank_pressure_history,
        State::Propulsion::tank_pressure);
}

static virtual_timer_t inner_tank_temperature_history_timer;
static virtual_timer_t outer_tank_temperature_history_timer;
static virtual_timer_t pressure_history_timer;

void DataCollection::initialize_propulsion_history_timers() {
    chVTObjectInit(&inner_tank_temperature_history_timer);
    chVTObjectInit(&outer_tank_temperature_history_timer);
    chVTObjectInit(&pressure_history_timer);

    rwMtxObjectInit(&StateHistory::Propulsion::propulsion_state_history_lock);

    chSysLock();
        chVTSet(&inner_tank_temperature_history_timer, DataCollectionRates::Propulsion::TANK_TEMPERATURE_INTERVAL, save_inner_tank_temperature_history, NULL);
        chVTSet(&outer_tank_temperature_history_timer, DataCollectionRates::Propulsion::TANK_TEMPERATURE_INTERVAL, save_outer_tank_temperature_history, NULL);
        chVTSet(&pressure_history_timer, DataCollectionRates::Propulsion::TANK_PRESSURE_INTERVAL, save_pressure_history, NULL);
    chSysUnlock();
}