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
    // Item: Master State Representation
    // Size: 4
    // Type: Integer
    // Minimum: 0
    // Maximum: 12
    // Description: Contains (master state + PAN state) represented as a 4-bit integer.
    std::bitset<4> master_state_representation(state_representation);
    
    for(int i = 0; i < master_state_representation.size(); i++)
        packet.set(packet_ptr++, master_state_representation[i]);
}

static void encode_last_uplink_data(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&State::Piksi::piksi_state_lock);
        // Item: Last Uplink Time
        // Size: 48
        // Type: GPS time
        // Description: Last time that a valid uplink was received from the ground.
        std::bitset<48> last_uplink_time_representation;
        trim_gps_time(State::Master::last_uplink_time, &last_uplink_time_representation);
    rwMtxRUnlock(&State::Piksi::piksi_state_lock);
    for(int i = 0; i < last_uplink_time_representation.size(); i++)
        packet.set(packet_ptr++, last_uplink_time_representation[i]);
    packet.set(packet_ptr++, State::Master::was_last_uplink_valid);
}

static void encode_hat(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    // Item: Hardware Availability Table
    // Type: Array of booleans
    // Size per element: 4
    // Number of elements: 11
    // Description: Hardware availability table. There are 11 devices; each device has the powered_on, enabled, is_functional, and error_ignored bits. See state/device_states.cpp for the order in which these bits are placed into the bitset.
    std::bitset<44> hat_representation;
    unsigned int hat_ptr = 0;
    rwMtxRLock(&State::Hardware::hat_lock);
        for(auto iter = State::Hardware::hat.begin(); iter != State::Hardware::hat.end(); ++iter) {
            hat_representation.set(hat_ptr++, iter->second.powered_on);
            hat_representation.set(hat_ptr++, iter->second.enabled);
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
        // Item: ADCS State Representation
        // Size: 4
        // Type: Integer
        // Minimum: 0
        // Maximum: 3
        // Description: Contains current ADCS controller state as a 2-bit integer.
        std::bitset<2> adcs_state_representation((unsigned char) State::ADCS::adcs_state);
    rwMtxRUnlock(&State::ADCS::adcs_state_lock);
    for(int i = 0; i < 2; i++)
        full_adcs_state_representation.set(i, adcs_state_representation[2-i]);
    rwMtxRLock(&State::ADCS::adcs_state_lock);
        // Item: Is propulsion pointing active?
        // Size: 1
        // Type: Boolean
        // Description: Indicates whether or not propulsion is currently setting the attitude of the satellite.
        full_adcs_state_representation.set(2, State::ADCS::is_propulsion_pointing_active);

        // Item: Is sun vector determination is working?
        // Size: 1
        // Type: Boolean
        // Description: Indicates whether or not the sun sensor data is sufficiently good for TRIAD to be providing us with a good atittude estimate.
        full_adcs_state_representation.set(3, State::ADCS::is_sun_vector_determination_working);
    rwMtxRUnlock(&State::ADCS::adcs_state_lock);
    
    for(int i = 0; i < full_adcs_state_representation.size(); i++)
        packet.set(packet_ptr++, full_adcs_state_representation[i]);
}

static void encode_adcs_hat(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    // Item: Hardware Availability Table
    // Type: Array of booleans
    // Size per element: 4
    // Number of elements: 17
    // Description: Hardware availability table for the ADCS. There are 17 devices; each device has the powered_on, enabled, is_functional, and error_ignored bits. See state/device_states.cpp for the order in which these bits are placed into the bitset.
    std::bitset<68> adcs_hat_representation;

    unsigned int hat_ptr = 0;
    rwMtxRLock(&State::Hardware::hat_lock);
    for(auto iter = State::ADCS::adcs_hat.begin(); iter != State::ADCS::adcs_hat.end(); ++iter) {
        adcs_hat_representation.set(hat_ptr++, iter->second.powered_on);
        adcs_hat_representation.set(hat_ptr++, iter->second.enabled);
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
        // Item: Is a propulsion manuever currently planned?
        // Size: 1
        // Type: Boolean
        // Description: Indicates whether or not the GNC algorithm has planned a propulsion manuever in the near future.
        propulsion_state_representation.set(0, State::Propulsion::is_firing_planned);
       
        // Item: Is a propulsion manuever currently planned?
        // Size: 1
        // Type: Boolean
        // Description: Indicates whether or not the propulsion system is currently repressurizing the tank in preparation for a maneuver.
        propulsion_state_representation.set(1, State::Propulsion::is_repressurization_active);
    rwMtxRUnlock(&State::Propulsion::propulsion_state_lock);

    for(int i = 0; i < propulsion_state_representation.size(); i++)
        packet.set(packet_ptr++, propulsion_state_representation[i]);
}

static void encode_propulsion_data(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    // Item: Remaining delta-v
    // Size: 11
    // Type: Float
    // Minimum: 0
    // Maximum: 15
    // Units: m/s
    // Description: Remaining delta-v, as computed by integrating the set of thrust vector firings that have
    // occurred so far.
    std::bitset<11> delta_v_representation;

    // Item: Thrust vector
    // Size: 26
    // Type: Vector
    // Minimum: 0
    // Maximum: 0.005
    // Units: m/s
    // Description: Thrust vector for the upcoming planned propulsion manuever, if any. If there is no propulsion manuever planned, this field's data is meaningless.
    std::bitset<26> thrust_vector_representation;

    // Item: Firing time
    // Size: 48
    // Type: GPS time
    // Description: Time of execution for the upcoming planned propulsion manuever, if any. If there is no propulsion manuever planned this field's data is meaningless.
    std::bitset<48> firing_time_representation;

    // Read propulsion data into state 
    rwMtxRLock(&State::Propulsion::propulsion_state_lock);
        // TODO integrate delta-v
        float delta_v_available = State::Propulsion::delta_v_available;
        std::array<float, 3> thrust_vector;
        for(int i = 0; i < 3; i++) thrust_vector[i] = State::Propulsion::firing_data.thrust_vector[i];
        gps_time_t firing_time;
        firing_time = State::Propulsion::firing_data.thrust_time;
    rwMtxRUnlock(&State::Propulsion::propulsion_state_lock);
    // Write into packet
    trim_float(delta_v_available, 0, 15, &delta_v_representation);
    for(int i = 0; i < delta_v_representation.size(); i++)
        packet.set(packet_ptr++, delta_v_representation[i]);
    trim_vector(thrust_vector, 0.005, &thrust_vector_representation);
    for(int i = 0; i < thrust_vector_representation.size(); i++)
        packet.set(packet_ptr++, thrust_vector_representation[i]);
    trim_gps_time(firing_time, &firing_time_representation);
    for(int i = 0; i < firing_time_representation.size(); i++)
        packet.set(packet_ptr++, firing_time_representation[i]);

    // Item: Outer Tank Pressure History
    // Size: 300
    // Type: Sequence of floats
    // Minimum: 0
    // Maximum: 100
    // Units: psi
    // Description: Outer tank pressure history. This is one time-averaged data point every 10 seconds from the last five minutes.
    rwMtxRLock(&StateHistory::Propulsion::propulsion_state_history_lock);
    while(!StateHistory::Propulsion::tank_pressure_history.empty()) {
        std::bitset<10> tank_pressure_representation;
        trim_float(StateHistory::Propulsion::tank_pressure_history.get(), 0, 100, &tank_pressure_representation);
        for(int i = 0; i < tank_pressure_representation.size(); i++)
            packet.set(packet_ptr++, tank_pressure_representation[i]);
    }
    rwMtxRUnlock(&StateHistory::Propulsion::propulsion_state_history_lock);

    // Item: Inner Tank Temperature History
    // Size: 300
    // Type: Sequence of floats
    // Minimum: -40
    // Maximum: 125
    // Units: C
    // Description: Inner tank temperature history. This is one time-averaged data point every 10 seconds from the last five minutes.
    rwMtxRLock(&StateHistory::Propulsion::propulsion_state_history_lock);
    while(!StateHistory::Propulsion::inner_tank_temperature_history.empty()) {
        std::bitset<9> inner_tank_temperature_representation;
        trim_temperature(StateHistory::Propulsion::inner_tank_temperature_history.get(), &inner_tank_temperature_representation);
        for(int i = 0; i < inner_tank_temperature_representation.size(); i++)
            packet.set(packet_ptr++, inner_tank_temperature_representation[i]);
    }
    rwMtxRUnlock(&StateHistory::Propulsion::propulsion_state_history_lock);

    // Item: Outer Tank Temperature History
    // Size: 300
    // Type: Sequence of floats
    // Minimum: -40
    // Maximum: 125
    // Units: C
    // Description: Outer tank temperature history. This is one time-averaged data point every 10 seconds from the last five minutes.
    rwMtxRLock(&StateHistory::Propulsion::propulsion_state_history_lock);
    while(!StateHistory::Propulsion::outer_tank_temperature_history.empty()) {
        std::bitset<9> outer_tank_temperature_representation;
        trim_temperature(StateHistory::Propulsion::outer_tank_temperature_history.get(), &outer_tank_temperature_representation);
        for(int i = 0; i < outer_tank_temperature_representation.size(); i++)
            packet.set(packet_ptr++, outer_tank_temperature_representation[i]);
    }
    rwMtxRUnlock(&StateHistory::Propulsion::propulsion_state_history_lock);
}

static void encode_gomspace_data(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    // Item: Battery voltage
    // Size: 11
    // Type: Integer
    // Minimum: 5000
    // Maximum: 9000
    // Units: mV
    // Description: Voltage of the battery.
    std::bitset<11> batt_voltage_representation;

    // Item: Boost converter voltages
    // Size per element: 12
    // Number of elements: 3
    // Type: Array of integers
    // Minimum: 0
    // Maximum: 4000
    // Units: mV
    // Description: Current of boost converter 1, 2, and 3.
    std::bitset<12> boost_converter_voltage_representation[3];

    // Item: Boost converter currents
    // Type: Array of integers
    // Size per element: 10
    // Number of elements: 3
    // Minimum: 0
    // Maximum: 1000
    // Units: mA
    // Description: Current of boost converter 1, 2, and 3.
    std::bitset<10> boost_converter_current[3];

    // Item: Output values
    // Type: Array of booleans
    // Size per element: 1
    // Number of elements: 6
    // Description: Whether or not outputs 1-6 are on. Output 7 is whether or not the heater is on.
    std::bitset<7> output_data;

    // Item: Output currents
    // Type: Array of integers
    // Size per element: 7
    // Number of elements: 6
    // Minimum: 0
    // Maximum: 1000
    // Units: mA
    // Description: Currents of outputs 1-6.
    std::bitset<7> output_current_data[6];

    // Item: WDT boots
    // Size: 32
    // Type: Integer
    // Minimum: 0
    // Maximum: 4294967295
    // Description: Number of reboots of satellite system due to Gomspace WDT.
    std::bitset<32> wdt_boots;

    // Item: Gomspace Battery Temperature 1
    // Size: 9
    // Type: Integer
    // Minimum: -40
    // Maximum: 125
    // Units: C
    // Description: Temperature 1 on Gomspace. // TODO figure out what this means?
    // Item: Gomspace Battery Temperature 2
    // Size: 9
    // Type: Integer
    // Minimum: -40
    // Maximum: 125
    // Units: C
    // Description: Temperature 2 on Gomspace. // TODO figure out what this means?
    std::bitset<9> temperatures[2];
    std::bitset<56> config;
    int config_ptr = 0;

    rwMtxRLock(&State::Gomspace::gomspace_state_lock);
        trim_int(State::Gomspace::gomspace_data.vbatt, 6000, 8000, &batt_voltage_representation);
        for(int i = 0; i < 3; i++)
            trim_int(State::Gomspace::gomspace_data.vboost[i], 0, 4000, &boost_converter_voltage_representation[i]);
        for(int i = 0; i < 3; i++)
            trim_int(State::Gomspace::gomspace_data.curin[i], 0, 1000, &boost_converter_current[i]);
        for(int i = 0; i < 6; i++)
            output_data.set(i, State::Gomspace::gomspace_data.output[i + 2]);
        // TODO heater output

        // TODO add output currents
        std::bitset<32> boots_from_wdt(State::Gomspace::gomspace_data.counter_wdt_i2c);
        wdt_boots = boots_from_wdt;
        for(int i = 0; i < 2; i++)
            trim_temperature(State::Gomspace::gomspace_data.temp[i], &temperatures[i]);
        
        // Config
        // Item: Gomspace PPT mode
        // Size: 1
        // Type: Boolean
        // Description: If 0, Maximum Power-Point Tracking. If 1, disabled
        config.set(config_ptr++, (State::Gomspace::gomspace_config.ppt_mode - 1));

        // Item: Gomspace Battery Heater mode
        // Size: 1
        // Type: Boolean
        // Description: If 0, manual, if 1, automatic.
        config.set(config_ptr++, State::Gomspace::gomspace_config.battheater_mode);

        // Item: Battery Heater "ON" temperature
        // Size: 9
        // Type: Float
        // Minimum: -40
        // Maximum: 125
        // Units: C
        // Description: The temperature threshold for automatically turning on the battery heater.
        std::bitset<9> batt_heater_on_temp;

        // Item: Battery Heater "OFF" temperature
        // Size: 9
        // Type: Float
        // Minimum: -40
        // Maximum: 125
        // Units: C
        // Description: The temperature threshold for automatically turning off the battery heater.
        std::bitset<9> batt_heater_off_temp;
        trim_temperature(State::Gomspace::gomspace_config.battheater_low, &batt_heater_on_temp);
        trim_temperature(State::Gomspace::gomspace_config.battheater_high, &batt_heater_off_temp);
        for(int i = 0; i < 9; i++)
            config.set(config_ptr++, batt_heater_on_temp[i]);
        for(int i = 0; i < 9; i++)
            config.set(config_ptr++, batt_heater_off_temp[i]);
        for(int i = 0; i < 8; i++)
            config.set(config_ptr++, State::Gomspace::gomspace_config.output_normal_value[i]);
        for(int i = 0; i < 8; i++)
            config.set(config_ptr++, State::Gomspace::gomspace_config.output_safe_value[i]);

        std::bitset<6> batt_voltage_config[4];
        // Item: Max Battery Voltage
        // Size: 6
        // Type: Integer
        // Minimum: 50
        // Maximum: 90
        // Units: 100 mV
        // Description: Battery voltage that is considered "maximum" by Gomspace.
        trim_int(State::Gomspace::gomspace_config2.batt_maxvoltage / 100, 50, 90, &batt_voltage_config[0]);
                // Item: Normal Battery Voltage
        // Size: 6
        // Type: Integer
        // Minimum: 50
        // Maximum: 90
        // Units: 100 mV
        // Description: Battery voltage that is considered "normal" by Gomspace.
        trim_int(State::Gomspace::gomspace_config2.batt_normalvoltage / 100, 50, 90, &batt_voltage_config[1]);
        // Item: Safe Battery Voltage
        // Size: 6
        // Type: Integer
        // Minimum: 50
        // Maximum: 90
        // Units: 100 mV
        // Description: Battery voltage that is considered "safe" by Gomspace.
        trim_int(State::Gomspace::gomspace_config2.batt_safevoltage / 100, 50, 90, &batt_voltage_config[2]);
        // Item: Critical Battery Voltage
        // Size: 6
        // Type: Integer
        // Minimum: 50
        // Maximum: 90
        // Units: 100 mV
        // Description: Battery voltage that is considered "critical" by Gomspace.
        trim_int(State::Gomspace::gomspace_config2.batt_criticalvoltage / 100, 50, 90, &batt_voltage_config[3]);
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
    // Item: Position history
    // Type: Array of vectors
    // Size per element: 62
    // Number of elements: 5
    // Minimum: 6400
    // Maximum: 7000
    // Units: km
    // Description: Position of satellite as a set of doubles in space.
    rwMtxRLock(&StateHistory::Piksi::piksi_state_history_lock);
    while(!StateHistory::Piksi::position_history.empty()) {
        std::bitset<62> position_representation;
        trim_vector(StateHistory::Piksi::position_history.get(), 6400.0, 7000.0, &position_representation);
        for(int i = 0; i < position_representation.size(); i++)
            packet.set(packet_ptr++, position_representation[i]);
    }
    rwMtxRUnlock(&StateHistory::Piksi::piksi_state_history_lock);

    // Item: Velocity history
    // Type: Array of vectors
    // Size per element: 62
    // Number of elements: 5
    // Minimum: 5
    // Maximum: 15
    // Units: km/s
    // Description: Position of satellite as a set of doubles in space.
    rwMtxRLock(&StateHistory::Piksi::piksi_state_history_lock);
    while(!StateHistory::Piksi::position_history.empty()) {
        std::bitset<62> velocity_representation;
        trim_vector(StateHistory::Piksi::velocity_history.get(), 5000.0, 9000.0, &velocity_representation);
        for(int i = 0; i < velocity_representation.size(); i++)
            packet.set(packet_ptr++, velocity_representation[i]);
    }
    rwMtxRUnlock(&StateHistory::Piksi::piksi_state_history_lock);
}

static void encode_piksi_time(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    // Item: Current time
    // Size: 48
    // Type: GPS time
    // Description: Current GPS time according to satellite.
    std::bitset<48> gps_time;
    rwMtxRLock(&State::Piksi::piksi_state_lock);
        gps_time_t current_time = State::Piksi::current_time;
    rwMtxRUnlock(&State::Piksi::piksi_state_lock);
    trim_gps_time(current_time, &gps_time);
    for(int i = 0; i < gps_time.size(); i++)
        packet.set(packet_ptr++, gps_time[i]);
}

static void encode_current_adcs_data(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    // Item: Current atittude
    // Size: 29
    // Type: GPS time
    // Description: Current satellite attitude in // TODO frame??
    std::bitset<29> attitude_representation;

    // Item: Current angular rate
    // Size: 30
    // Type: GPS time
    // Description: Current satellite angular rate in // TODO frame??
    std::bitset<30> rate_representation;

    rwMtxRLock(&State::ADCS::adcs_state_lock);
        std::array<float, 4> cur_attitude = State::ADCS::cur_attitude;
        std::array<float, 3> cur_ang_rate = State::ADCS::cur_ang_rate;
    rwMtxRUnlock(&State::ADCS::adcs_state_lock);
    trim_quaternion(cur_attitude, &attitude_representation);
    trim_vector(cur_ang_rate, 0, 5, &rate_representation); // TODO check numbers
    for(int i = 0; i < attitude_representation.size(); i++)
        packet.set(packet_ptr++, attitude_representation[i]);
    for(int i = 0; i < rate_representation.size(); i++)
        packet.set(packet_ptr++, rate_representation[i]);
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
