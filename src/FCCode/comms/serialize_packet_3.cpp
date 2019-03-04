/** @file serialize_packet_3.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for functions that serialize the third downlink packet.
 */

#include "downlink_serializer.hpp"

using namespace Comms;

static void encode_gyroscope_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        // Item: Gyroscope data history
        // Type: Array of vectors
        // Size per element: 32
        // Number of elements: 15
        // Description: Record of gyroscope data. One time-averaged value every 20 seconds from the last 5 minutes
        while(!StateHistory::ADCS::gyro_history.empty()) {
            std::array<float, 3> gyro_data = StateHistory::ADCS::gyro_history.get();
            // Split gyro data into 3 components since we care more about the Z component
            std::bitset<10> gyro_x_axis;
            std::bitset<10> gyro_y_axis;
            std::bitset<12> gyro_z_axis;
            trim_float(gyro_data[0], 
                Constants::ADCS::MIN_GYRO_VALUE, Constants::ADCS::MAX_GYRO_VALUE, &gyro_x_axis);
            trim_float(gyro_data[1], 
                Constants::ADCS::MIN_GYRO_VALUE, Constants::ADCS::MAX_GYRO_VALUE, &gyro_y_axis);
            trim_float(gyro_data[2], 
                Constants::ADCS::MIN_GYRO_VALUE, Constants::ADCS::MAX_GYRO_VALUE, &gyro_z_axis);
            for(unsigned int i = 0; i < gyro_x_axis.size(); i++)
                packet.set(packet_ptr++, gyro_x_axis[i]);
            for(unsigned int i = 0; i < gyro_y_axis.size(); i++)
                packet.set(packet_ptr++, gyro_y_axis[i]);
            for(unsigned int i = 0; i < gyro_z_axis.size(); i++)
                packet.set(packet_ptr++, gyro_z_axis[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

static void encode_magnetometer_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        // Item: Magnetometer history
        // Type: Array of vectors
        // Size per element: 29
        // Number of elements: 20
        // Description: Record of gyroscope data. One time-averaged value every 15 seconds from the last 5 minutes
        while(!StateHistory::ADCS::magnetometer_history.empty()) {
            std::array<float, 3> mag = StateHistory::ADCS::magnetometer_history.get();
            std::bitset<29> magnetometer_representation;
            trim_vector(mag,
                Constants::ADCS::MAX_MAGNETOMETER_READING, 
                &magnetometer_representation);
            for(unsigned int i = 0; i < magnetometer_representation.size(); i++)
                packet.set(packet_ptr++, magnetometer_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

static void encode_rwa_ramp_command_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        // Item: Reaction wheel ramp command history
        // Type: Array of vectors
        // Size per element: 29
        // Number of elements: 10
        // Description: Record of reaction wheel ramp commands. One instantaneous value every 30 seconds from the last 5 minutes
        while(!StateHistory::ADCS::rwa_ramp_cmd_history.empty()) {
            std::array<float, 3> ramp_cmd = StateHistory::ADCS::rwa_ramp_cmd_history.get();
            std::bitset<29> ramp_cmd_representation;
            trim_vector(ramp_cmd,
                Constants::ADCS::MAX_RAMP_CMD, 
                &ramp_cmd_representation);
            for(unsigned int i = 0; i < ramp_cmd_representation.size(); i++)
                packet.set(packet_ptr++, ramp_cmd_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

static void encode_mtr_command_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        // Item: Magnetorquer command history
        // Type: Array of vectors
        // Size per element: 32
        // Number of elements: 10
        // Description: Record of magnetorquer commands. One instantaneous value every 30 seconds from the last 5 minutes
        while(!StateHistory::ADCS::mtr_cmd_history.empty()) {
            std::array<float, 3> mtr_cmd = StateHistory::ADCS::mtr_cmd_history.get();
            std::bitset<29> mtr_cmd_representation;
            trim_vector(mtr_cmd, Constants::ADCS::MAX_MTR_CMD, &mtr_cmd_representation);
            for(unsigned int i = 0; i < mtr_cmd_representation.size(); i++)
                packet.set(packet_ptr++, mtr_cmd_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

static void encode_ssa_vec_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        // Item: Sun sensor vector history
        // Type: Array of vectors
        // Size per element: 21
        // Number of elements: 10
        // Description: Record of sun sensor vector values. One instantaneous value every 30 seconds from the last 5 minutes
        while(!StateHistory::ADCS::ssa_vector_history.empty()) {
            std::array<float, 3> ssa_vec = StateHistory::ADCS::ssa_vector_history.get();
            std::bitset<21> ssa_vec_representation;
            trim_vector(ssa_vec, 1.0f, &ssa_vec_representation);
            for(unsigned int i = 0; i < ssa_vec_representation.size(); i++)
                packet.set(packet_ptr++, ssa_vec_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

static void encode_ssa_adc_data(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        // Item: Sun sensor array data
        // Type: Array of bytes
        // Size per element: 8
        // Number of elements: 20
        // Description: Record of current sun sensor phototransistor values.
        std::bitset<8> ssa_adc_data_representation[20];
        for(int i = 0; i < 20; i++)
            trim_float(State::ADCS::ssa_adc_data[i], 
                Constants::ADCS::MIN_SUN_SENSOR_VALUE, 
                Constants::ADCS::MAX_SUN_SENSOR_VALUE,
                &ssa_adc_data_representation[i]);
        for(int j = 0; j < 20; j++) {
            for(unsigned int i = 0; i < ssa_adc_data_representation[j].size(); i++)
                packet.set(packet_ptr++, ssa_adc_data_representation[j][i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

void Comms::serialize_packet_3(std::array<char, PACKET_SIZE_BYTES>& dest) {
    std::bitset<PACKET_SIZE_BITS> packet;
    unsigned int packet_ptr = 0;
    
    encode_gyroscope_history(packet, packet_ptr);
    encode_magnetometer_history(packet, packet_ptr);
    encode_rwa_ramp_command_history(packet, packet_ptr);
    encode_mtr_command_history(packet, packet_ptr);
    encode_ssa_vec_history(packet, packet_ptr);
    encode_ssa_adc_data(packet, packet_ptr);

    //add_packet_checksum(packet, packet_ptr, &dest);
}