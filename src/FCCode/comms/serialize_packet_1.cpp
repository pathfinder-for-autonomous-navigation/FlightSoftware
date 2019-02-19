/** @file serialize_packet_1.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for functions that serialize the first downlink packet.
 */

#include "downlink_serializer.hpp"

using namespace Comms;

static void encode_master_state(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&State::Master::master_state_lock);
        State::Master::MasterState master_state = State::Master::master_state;
        State::Master::PANState pan_state = State::Master::pan_state;
    rwMtxRUnlock(&State::Master::master_state_lock);
    unsigned char state_representation = (unsigned char) master_state;
    if (master_state == State::Master::MasterState::NORMAL) state_representation = (unsigned char) pan_state + 4;
    std::bitset<4> master_state_representation(state_representation);
    
    for(int i = 0; i < master_state_representation.size(); i++)
        packet.set(packet_ptr++, master_state_representation[i]);
}

static void encode_last_uplink_data(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&State::Piksi::piksi_state_lock);
        std::bitset<48> last_uplink_time;
        trim_gps_time(State::Master::last_uplink_time, &last_uplink_time);
    rwMtxRUnlock(&State::Piksi::piksi_state_lock);
    for(int i = 0; i < last_uplink_time.size(); i++)
        packet.set(packet_ptr++, last_uplink_time[i]);
    packet.set(packet_ptr++, State::Master::was_last_uplink_valid);
}

static void encode_hat(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    std::bitset<33> hat_representation;
    unsigned int hat_ptr = 0;
    rwMtxRLock(&State::Hardware::hat_lock);
        for(auto iter = State::Hardware::hat.begin(); iter != State::Hardware::hat.end(); ++iter) {
            hat_representation.set(hat_ptr++, iter->second.powered_on);
            hat_representation.set(hat_ptr++, iter->second.is_functional);
            hat_representation.set(hat_ptr++, iter->second.error_ignored);
        }
    rwMtxRUnlock(&State::Hardware::hat_lock);

    for(int i = 0; i < hat_representation.size(); i++)
        packet.set(packet_ptr++, hat_representation[i]);
}

static void encode_adcs_state(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    std::bitset<4> full_adcs_state_representation;
    rwMtxRLock(&State::ADCS::adcs_state_lock);
        std::bitset<2> adcs_state_representation((unsigned char) State::ADCS::adcs_state);
    rwMtxRUnlock(&State::ADCS::adcs_state_lock);
    for(int i = 0; i < 2; i++)
        full_adcs_state_representation.set(i, adcs_state_representation[2-i]);
    rwMtxRLock(&State::ADCS::adcs_state_lock);
        full_adcs_state_representation.set(2, State::ADCS::is_propulsion_pointing_active);
        full_adcs_state_representation.set(3, State::ADCS::is_sun_vector_determination_working);
    rwMtxRUnlock(&State::ADCS::adcs_state_lock);
    
    for(int i = 0; i < full_adcs_state_representation.size(); i++)
        packet.set(packet_ptr++, full_adcs_state_representation[i]);
}

static void encode_adcs_hat(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    std::bitset<34> adcs_hat_representation;

    unsigned int hat_ptr = 0;
    rwMtxRLock(&State::Hardware::hat_lock);
    for(auto iter = State::ADCS::adcs_hat.begin(); iter != State::ADCS::adcs_hat.end(); ++iter) {
        adcs_hat_representation.set(hat_ptr++, iter->second.is_functional);
        adcs_hat_representation.set(hat_ptr++, iter->second.error_ignored);
    }
    rwMtxRUnlock(&State::Hardware::hat_lock);

    for(int i = 0; i < adcs_hat_representation.size(); i++)
        packet.set(packet_ptr++, adcs_hat_representation[i]);
}

static void encode_propulsion_state(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    std::bitset<2> propulsion_state_representation;

    rwMtxRLock(&State::Propulsion::propulsion_state_lock);
        propulsion_state_representation.set(0, State::Propulsion::is_firing_planned);
        propulsion_state_representation.set(1, State::Propulsion::is_repressurization_active);
    rwMtxRUnlock(&State::Propulsion::propulsion_state_lock);

    for(int i = 0; i < propulsion_state_representation.size(); i++)
        packet.set(packet_ptr++, propulsion_state_representation[i]);
}

static void encode_propulsion_data(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    std::bitset<11> delta_v_representation;
    std::bitset<30> thrust_vector_representation;
    std::bitset<48> firing_time_representation;

    rwMtxRLock(&State::Propulsion::propulsion_state_lock);
        // Remaining delta-V
        // TODO integrate delta-v
        trim_float(State::Propulsion::delta_v_available, 0, 15, &delta_v_representation);
        // Thrust vector
        std::array<float, 3> thrust_vector;
        for(int i = 0; i < 3; i++) thrust_vector[i] = State::Propulsion::firing_data.thrust_vector[i];
        trim_vector(thrust_vector, 2, &thrust_vector_representation);
        // Firing time
        msg_gps_time_t firing_time;
        firing_time.wn = State::Propulsion::firing_data.thrust_time.wn;
        firing_time.tow = State::Propulsion::firing_data.thrust_time.tow;
        trim_gps_time(firing_time, &firing_time_representation);
    rwMtxRUnlock(&State::Propulsion::propulsion_state_lock);
    
    rwMtxRLock(&StateHistory::Propulsion::propulsion_state_history_lock);
        // Tank pressure history
        while(!StateHistory::Propulsion::tank_pressure_history.empty()) {
            std::bitset<10> tank_pressure_representation;
            trim_float(StateHistory::Propulsion::tank_pressure_history.get(), 0, 100, &tank_pressure_representation);
            for(int i = 0; i < tank_pressure_representation.size(); i++)
                packet.set(packet_ptr++, tank_pressure_representation[i]);
        }
        // Inner tank temperature history
        while(!StateHistory::Propulsion::inner_tank_temperature_history.empty()) {
            std::bitset<9> inner_tank_temperature_representation;
            trim_temperature(StateHistory::Propulsion::inner_tank_temperature_history.get(), &inner_tank_temperature_representation);
            for(int i = 0; i < inner_tank_temperature_representation.size(); i++)
                packet.set(packet_ptr++, inner_tank_temperature_representation[i]);
        }
        // Inner tank temperature history
        while(!StateHistory::Propulsion::outer_tank_temperature_history.empty()) {
            std::bitset<9> outer_tank_temperature_representation;
            trim_temperature(StateHistory::Propulsion::outer_tank_temperature_history.get(), &outer_tank_temperature_representation);
            for(int i = 0; i < outer_tank_temperature_representation.size(); i++)
                packet.set(packet_ptr++, outer_tank_temperature_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::Propulsion::propulsion_state_history_lock);

    for(int i = 0; i < delta_v_representation.size(); i++)
        packet.set(packet_ptr++, delta_v_representation[i]);
    for(int i = 0; i < thrust_vector_representation.size(); i++)
        packet.set(packet_ptr++, thrust_vector_representation[i]);
    for(int i = 0; i < firing_time_representation.size(); i++)
        packet.set(packet_ptr++, firing_time_representation[i]);
}

static void encode_gomspace_data(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    static constexpr int section_size = 187;
    std::bitset<section_size> section_representation;

    std::bitset<11> batt_voltage_representation;
    std::bitset<12> boost_converter_voltage_representation[3];
    std::bitset<10> boost_converter_current[3];
    std::bitset<6> output_data;
    std::bitset<32> wdt_boots;
    std::bitset<9> temperatures[2];
    std::bitset<56> config; // TODO double check bit counts below. Something went wrong in specification.
    int config_ptr = 0;

    // TODO check limits on all of these
    rwMtxRLock(&State::Gomspace::gomspace_state_lock);
        // Battery voltage
        trim_int(State::Gomspace::gomspace_data.vbatt, 6000, 8000, &batt_voltage_representation);
        // Boost converter voltage
        for(int i = 0; i < 3; i++)
            trim_int(State::Gomspace::gomspace_data.vboost[i], 0, 4000, &boost_converter_voltage_representation[i]);
        // Boost converter current
        for(int i = 0; i < 3; i++)
            trim_int(State::Gomspace::gomspace_data.curin[i], 0, 1000, &boost_converter_current[i]);
        // Output values
        for(int i = 0; i < 6; i++)
            output_data.set(i, State::Gomspace::gomspace_data.output[i + 2]);
        // TODO add output currents
        // Boots due to WDT timeout
        std::bitset<32> boots_from_wdt(State::Gomspace::gomspace_data.counter_wdt_i2c);
        wdt_boots = boots_from_wdt;
        // Temperature data
        for(int i = 0; i < 2; i++)
            trim_temperature(State::Gomspace::gomspace_data.temp[i], &temperatures[i]);
        // Config
        // PPT Mode
        config.set(config_ptr++, State::Gomspace::gomspace_config.ppt_mode);
        // Battery heating 
        config.set(config_ptr++, State::Gomspace::gomspace_config.battheater_mode);
        std::bitset<9> batt_heater_on_temp;
        std::bitset<9> batt_heater_off_temp;
        trim_temperature(State::Gomspace::gomspace_config.battheater_low, &batt_heater_on_temp);
        trim_temperature(State::Gomspace::gomspace_config.battheater_high, &batt_heater_off_temp);
        for(int i = 0; i < 9; i++)
            config.set(config_ptr++, batt_heater_on_temp[i]);
        for(int i = 0; i < 9; i++)
            config.set(config_ptr++, batt_heater_off_temp[i]);
        // Normal- and safe-mode output values
        for(int i = 0; i < 8; i++)
            config.set(config_ptr++, State::Gomspace::gomspace_config.output_normal_value[i]);
        for(int i = 0; i < 8; i++)
            config.set(config_ptr++, State::Gomspace::gomspace_config.output_safe_value[i]);
        // Battery voltage configurations
        std::bitset<5> batt_voltage_config[4];
        trim_int(State::Gomspace::gomspace_config2.batt_maxvoltage / 100, 60, 80, &batt_voltage_config[0]);
        trim_int(State::Gomspace::gomspace_config2.batt_normalvoltage / 100, 60, 80, &batt_voltage_config[1]);
        trim_int(State::Gomspace::gomspace_config2.batt_safevoltage / 100, 60, 80, &batt_voltage_config[2]);
        trim_int(State::Gomspace::gomspace_config2.batt_criticalvoltage / 100, 60, 80, &batt_voltage_config[3]);
        for(int j = 0; j < 4; j++) {
            for(int i = 0; i < batt_voltage_config[0].size(); i++)
                config.set(config_ptr++, batt_voltage_config[j][i]);
        }
    rwMtxRUnlock(&State::Gomspace::gomspace_state_lock);

    for(int i = 0; i < batt_voltage_representation.size(); i++)
        packet.set(packet_ptr++, batt_voltage_representation[i]);
    for(int j = 0; j < 3; j++) {
        for(int i = 0; i < boost_converter_voltage_representation[0].size(); i++)
            packet.set(packet_ptr++, boost_converter_voltage_representation[j][i]);
        for(int i = 0; i < boost_converter_current[0].size(); i++)
            packet.set(packet_ptr++, boost_converter_current[j][i]);
    }
    for(int i = 0; i < output_data.size(); i++)
        packet.set(packet_ptr++, output_data[i]);
    for(int i = 0; i < wdt_boots.size(); i++)
        packet.set(packet_ptr++, wdt_boots[i]);
    for(int j = 0; j < 2; j++) {
        for(int i = 0; i < temperatures[0].size(); i++)
            packet.set(packet_ptr++, temperatures[j][i]);
    }
    for(int i = 0; i < config.size(); i++)
        packet.set(packet_ptr++, config[i]);
}

static void encode_piksi_data_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::Piksi::piksi_state_history_lock);
        while(!StateHistory::Piksi::position_history.empty()) {
            std::bitset<37> position_history_representation;
            trim_vector(StateHistory::Piksi::position_history.get(), 6400, 7000, &position_history_representation);
            for(int i = 0; i < position_history_representation.size(); i++)
                packet.set(packet_ptr++, position_history_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::Piksi::piksi_state_history_lock);
}

static void encode_piksi_time(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&State::Piksi::piksi_state_lock);
        std::bitset<48> gps_time;
        trim_gps_time(State::Piksi::current_time, &gps_time);
    rwMtxRUnlock(&State::Piksi::piksi_state_lock);
    for(int i = 0; i < gps_time.size(); i++)
        packet.set(packet_ptr++, gps_time[i]);
}

static void encode_current_adcs_data(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    std::bitset<29> trimmed_attitude;
    std::bitset<30> trimmed_rate;
    rwMtxRLock(&State::ADCS::adcs_state_lock);
        trim_quaternion(State::ADCS::cur_attitude, &trimmed_attitude);
        trim_vector(State::ADCS::cur_ang_rate, 0, 5, &trimmed_rate); // TODO check numbers
    rwMtxRUnlock(&State::ADCS::adcs_state_lock);
    for(int i = 0; i < trimmed_attitude.size(); i++)
        packet.set(packet_ptr++, trimmed_attitude[i]);
    for(int i = 0; i < trimmed_rate.size(); i++)
        packet.set(packet_ptr++, trimmed_rate[i]);
}

void Comms::serialize_packet_1(std::array<char, PACKET_SIZE_BYTES>& dest) {
    std::bitset<PACKET_SIZE_BITS> packet;
    unsigned int packet_ptr = 0;
    
    encode_master_state(packet, packet_ptr);
    encode_last_uplink_data(packet, packet_ptr);
    encode_hat(packet, packet_ptr);
    encode_adcs_state(packet, packet_ptr);
    encode_adcs_hat(packet, packet_ptr);
    encode_propulsion_state(packet, packet_ptr);
    encode_propulsion_data(packet, packet_ptr);
    encode_gomspace_data(packet, packet_ptr);
    encode_piksi_data_history(packet, packet_ptr);
    encode_piksi_time(packet, packet_ptr);
    encode_current_adcs_data(packet, packet_ptr);

    //add_packet_checksum(packet, packet_ptr, &dest);
}
