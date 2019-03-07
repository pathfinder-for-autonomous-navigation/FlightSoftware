/** @file adcs_data.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains timers that periodically collect ADCS data and store it into a state buffer.
 */

#include <AttitudeEstimator.hpp>
#include "../state/state_holder.hpp"
#include "../state/state_history_holder.hpp"
#include <circular_buffer.hpp>
#include "helpers.hpp"
#include "data_collection.hpp"
#include "data_collection_rates.hpp"

static virtual_timer_t gyroscope_history_timer;
static virtual_timer_t gyroscope_history_fast_and_avg_timer;
static virtual_timer_t attitude_command_history_timer;
static virtual_timer_t attitude_history_timer;
static virtual_timer_t attitude_history_fast_timer;
static virtual_timer_t attitude_history_avg_timer;
static virtual_timer_t rate_history_timer;
static virtual_timer_t rate_history_fast_timer;
static virtual_timer_t rate_history_avg_timer;
static virtual_timer_t spacecraft_L_history_timer;
static virtual_timer_t spacecraft_L_history_fast_timer;
static virtual_timer_t spacecraft_L_history_avg_timer;
static virtual_timer_t magnetometer_history_timer;
static virtual_timer_t magnetometer_history_avg_timer;
static virtual_timer_t rwa_ramp_cmd_timer;
static virtual_timer_t mtr_cmd_timer;
static virtual_timer_t ssa_vector_timer;
static virtual_timer_t ssa_array_timer;

static std::array<float, 3> avg_gyro;
static unsigned int num_gyro_measurements = 0;
static void save_gyro_history(void* arg) {
    DataCollection::add_to_buffer<std::array<float,3>,DataCollectionRates::ADCS::GYRO>(StateHistory::ADCS::gyro_history, avg_gyro);
    DataCollection::reset_avg<3>(&avg_gyro, &num_gyro_measurements);
    chSysLockFromISR();
        chVTSetI(&gyroscope_history_timer, MS2ST(DataCollectionRates::ADCS::GYRO_INTERVAL), save_gyro_history, NULL);
    chSysUnlockFromISR();
}
static void save_gyro_history_fast_and_avg(void* arg) { 
    DataCollection::add_to_buffer<std::array<float,3>,DataCollectionRates::ADCS::GYRO_FAST>(StateHistory::ADCS::gyro_fast_history, State::ADCS::gyro_data);
    DataCollection::add_to_avg(avg_gyro, State::ADCS::gyro_data, &num_gyro_measurements);
}

static void save_attitude_cmd_history(void* arg) {
    DataCollection::add_to_buffer<std::array<float,4>,DataCollectionRates::ADCS::ATTITUDE_CMD>(StateHistory::ADCS::attitude_cmd_history, State::ADCS::cmd_attitude);
}

static std::array<float, 4> avg_attitude;
static unsigned int num_attitude_measurements = 0;
static void save_atittude_history(void* arg) { 
    DataCollection::add_to_buffer<std::array<float,4>,DataCollectionRates::ADCS::ATTITUDE>(StateHistory::ADCS::attitude_history, avg_attitude); 
    DataCollection::reset_avg<4>(&avg_attitude, &num_attitude_measurements);
}
static void save_attitude_history_fast(void* arg) { 
    DataCollection::add_to_buffer<std::array<float,4>,DataCollectionRates::ADCS::ATTITUDE_FAST>(StateHistory::ADCS::attitude_fast_history, State::ADCS::cur_attitude);
}
static void attitude_history_avg(void* arg) {
    // TODO figure out how to average quaternions
}

static std::array<float, 3> avg_rate;
static unsigned int num_rate_measurements = 0;
static void save_rate_history(void* arg) {
    DataCollection::add_to_buffer<std::array<float,3>,DataCollectionRates::ADCS::RATE>(StateHistory::ADCS::rate_history, avg_rate); 
    DataCollection::reset_avg<3>(&avg_rate, &num_rate_measurements);
}
static void save_rate_history_fast(void* arg) { 
    DataCollection::add_to_buffer<std::array<float,3>,DataCollectionRates::ADCS::RATE_FAST>(StateHistory::ADCS::rate_fast_history, State::ADCS::cur_ang_rate); 
}
static void rate_history_avg(void* arg) { 
    DataCollection::add_to_avg(avg_gyro, State::ADCS::gyro_data, &num_gyro_measurements);
}

static std::array<float, 3> avg_spacecraft_L;
static unsigned int num_spacecraft_L_measurements = 0;
static void save_spacecraft_L_history(void* arg) { 
    DataCollection::add_to_buffer<std::array<float,3>,DataCollectionRates::ADCS::L>(StateHistory::ADCS::spacecraft_L_history, avg_spacecraft_L);
    DataCollection::reset_avg<3>(&avg_spacecraft_L, &num_spacecraft_L_measurements);
}
static void save_spacecraft_L_history_fast(void* arg) {
    std::array<float, 3> spacecraft_L;
    for(int i = 0; i < 3; i++) spacecraft_L[i] = ADCSControllers::Estimator::hwheel_sensor_body[i];
    DataCollection::add_to_buffer<std::array<float,3>,DataCollectionRates::ADCS::L_FAST>(StateHistory::ADCS::spacecraft_L_fast_history, spacecraft_L);
}
static void spacecraft_L_history_avg(void* arg) { 
    std::array<float, 3> spacecraft_L;
    for(int i = 0; i < 3; i++) spacecraft_L[i] = ADCSControllers::Estimator::hwheel_sensor_body[i];
    DataCollection::add_to_avg(avg_spacecraft_L, spacecraft_L, &num_spacecraft_L_measurements);
}

static std::array<float, 3> avg_magnetic_field;
static unsigned int num_mag_field_measurements = 0;
static void save_magnetometer_history(void* arg) { 
    DataCollection::add_to_buffer<std::array<float,3>,DataCollectionRates::ADCS::MAGNETOMETER>(StateHistory::ADCS::magnetometer_history, State::ADCS::mag_data);
    DataCollection::reset_avg<3>(&avg_magnetic_field, &num_mag_field_measurements);
}

static void magnetometer_history_avg(void* arg) { 
    DataCollection::add_to_avg(avg_magnetic_field, State::ADCS::mag_data, &num_mag_field_measurements);
}

static void save_rwa_ramp_cmd_history(void* arg) {
    DataCollection::add_to_buffer<std::array<float,3>,DataCollectionRates::ADCS::RWA_RAMP_CMD>(StateHistory::ADCS::rwa_ramp_cmd_history, State::ADCS::rwa_ramps);
}

static void save_mtr_cmd_history(void* arg) {
    DataCollection::add_to_buffer<std::array<float,3>,DataCollectionRates::ADCS::MTR_CMD>(StateHistory::ADCS::rwa_ramp_cmd_history, State::ADCS::rwa_ramps);
}

static void save_ssa_vector_history(void* arg) {
    DataCollection::add_to_buffer<std::array<float,3>,DataCollectionRates::ADCS::SSA_VECTOR>(StateHistory::ADCS::ssa_vector_history, State::ADCS::ssa_vec);
}

void DataCollection::initialize_adcs_history_timers() {
    chVTObjectInit(&gyroscope_history_timer);
    chVTObjectInit(&gyroscope_history_fast_and_avg_timer);
    chVTObjectInit(&attitude_command_history_timer);
    chVTObjectInit(&attitude_history_timer);
    chVTObjectInit(&attitude_history_fast_timer);
    chVTObjectInit(&attitude_history_avg_timer);
    chVTObjectInit(&rate_history_timer);
    chVTObjectInit(&rate_history_fast_timer);
    chVTObjectInit(&rate_history_avg_timer);
    chVTObjectInit(&spacecraft_L_history_timer);
    chVTObjectInit(&spacecraft_L_history_fast_timer);
    chVTObjectInit(&spacecraft_L_history_avg_timer);
    chVTObjectInit(&magnetometer_history_timer);
    chVTObjectInit(&magnetometer_history_avg_timer);
    chVTObjectInit(&rwa_ramp_cmd_timer);
    chVTObjectInit(&mtr_cmd_timer);
    chVTObjectInit(&ssa_vector_timer);
    chVTObjectInit(&ssa_array_timer);

    rwMtxObjectInit(&StateHistory::ADCS::adcs_history_state_lock);

    chSysLock();
        chVTSet(&gyroscope_history_timer, MS2ST(DataCollectionRates::ADCS::GYRO_INTERVAL), save_gyro_history, NULL);
        chVTSet(&gyroscope_history_fast_and_avg_timer, MS2ST(DataCollectionRates::ADCS::GYRO_FAST_INTERVAL), save_gyro_history_fast_and_avg, NULL);
        chVTSet(&attitude_command_history_timer, MS2ST(DataCollectionRates::ADCS::ATTITUDE_CMD_INTERVAL), save_attitude_cmd_history, NULL);
        chVTSet(&attitude_history_timer, MS2ST(DataCollectionRates::ADCS::ATTITUDE_INTERVAL), save_atittude_history, NULL);
        chVTSet(&attitude_history_fast_timer, MS2ST(DataCollectionRates::ADCS::ATTITUDE_FAST_INTERVAL), save_attitude_history_fast, NULL);
        chVTSet(&attitude_history_avg_timer, MS2ST(DataCollectionRates::ADCS::ATTITUDE_AVG_INTERVAL), attitude_history_avg, NULL);
        chVTSet(&rate_history_timer, MS2ST(DataCollectionRates::ADCS::RATE_INTERVAL), save_rate_history, NULL);
        chVTSet(&rate_history_fast_timer, MS2ST(DataCollectionRates::ADCS::RATE_FAST_INTERVAL), save_rate_history_fast, NULL);
        chVTSet(&rate_history_avg_timer, MS2ST(DataCollectionRates::ADCS::RATE_AVG_INTERVAL), rate_history_avg, NULL);
        chVTSet(&spacecraft_L_history_timer, MS2ST(DataCollectionRates::ADCS::L_INTERVAL), save_spacecraft_L_history, NULL);
        chVTSet(&spacecraft_L_history_fast_timer, MS2ST(DataCollectionRates::ADCS::L_FAST_INTERVAL), save_spacecraft_L_history_fast, NULL);
        chVTSet(&spacecraft_L_history_avg_timer, MS2ST(DataCollectionRates::ADCS::L_AVG_INTERVAL), spacecraft_L_history_avg, NULL);
        chVTSet(&magnetometer_history_timer, MS2ST(DataCollectionRates::ADCS::MAGNETOMETER_INTERVAL), save_magnetometer_history, NULL);
        chVTSet(&magnetometer_history_avg_timer, MS2ST(DataCollectionRates::ADCS::MAGNETOMETER_AVG_INTERVAL), magnetometer_history_avg, NULL);
        chVTSet(&rwa_ramp_cmd_timer, MS2ST(DataCollectionRates::ADCS::RWA_RAMP_CMD_INTERVAL), save_rwa_ramp_cmd_history, NULL);
        chVTSet(&mtr_cmd_timer, MS2ST(DataCollectionRates::ADCS::MTR_CMD_INTERVAL), save_mtr_cmd_history, NULL);
        chVTSet(&ssa_vector_timer, MS2ST(DataCollectionRates::ADCS::SSA_VECTOR_INTERVAL), save_ssa_vector_history, NULL);
    chSysUnlock();
}