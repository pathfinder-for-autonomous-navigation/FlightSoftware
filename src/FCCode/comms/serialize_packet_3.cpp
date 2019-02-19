/** @file serialize_packet_3.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for functions that serialize the third downlink packet.
 */

#include "downlink_serializer.hpp"

using namespace Comms;

static void encode_gyroscope_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        while(!StateHistory::ADCS::gyro_history.empty()) {
            std::array<float, 3> rate = StateHistory::ADCS::gyro_history.get();
            std::bitset<10> gyro_x_axis;
            std::bitset<10> gyro_y_axis;
            std::bitset<12> gyro_z_axis;
            trim_float(rate[0], -10, 10, &gyro_x_axis); // TODO fix numbers
            trim_float(rate[1], -10, 10, &gyro_y_axis); // TODO fix numbers
            trim_float(rate[2], -10, 10, &gyro_z_axis); // TODO fix numbers
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
        while(!StateHistory::ADCS::magnetometer_history.empty()) {
            std::array<float, 3> mag = StateHistory::ADCS::magnetometer_history.get();
            std::bitset<29> magnetometer_representation;
            trim_vector(mag, -10, 10, &magnetometer_representation); // TODO fix numbers
            for(unsigned int i = 0; i < magnetometer_representation.size(); i++)
                packet.set(packet_ptr++, magnetometer_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

static void encode_rwa_ramp_command_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        while(!StateHistory::ADCS::rwa_ramp_cmd_history.empty()) {
            std::array<float, 3> ramp_cmd = StateHistory::ADCS::rwa_ramp_cmd_history.get();
            std::bitset<29> ramp_cmd_representation;
            trim_vector(ramp_cmd, -10, 10, &ramp_cmd_representation); // TODO fix numbers
            for(unsigned int i = 0; i < ramp_cmd_representation.size(); i++)
                packet.set(packet_ptr++, ramp_cmd_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

static void encode_mtr_command_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        while(!StateHistory::ADCS::mtr_cmd_history.empty()) {
            std::array<float, 3> mtr_cmd = StateHistory::ADCS::mtr_cmd_history.get();
            std::bitset<29> mtr_cmd_representation;
            trim_vector(mtr_cmd, -10, 10, &mtr_cmd_representation); // TODO fix numbers
            for(unsigned int i = 0; i < mtr_cmd_representation.size(); i++)
                packet.set(packet_ptr++, mtr_cmd_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

// TODO fix since SSA vector is just a unit vector
static void encode_ssa_vec_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        while(!StateHistory::ADCS::ssa_vector_history.empty()) {
            std::array<float, 3> ssa_vec = StateHistory::ADCS::ssa_vector_history.get();
            std::bitset<29> ssa_vec_representation;
            trim_vector(ssa_vec, -10, 10, &ssa_vec_representation); // TODO fix numbers
            for(unsigned int i = 0; i < ssa_vec_representation.size(); i++)
                packet.set(packet_ptr++, ssa_vec_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

static void encode_ssa_adc_data(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        std::bitset<8> ssa_adc_data_representation[20];
        for(int i = 0; i < 20; i++)
            trim_float(State::ADCS::ssa_adc_data[i], -10, 10, &ssa_adc_data_representation[i]); // TODO fix numbers
        for(int j = 0; j < 20; j++) {
            for(unsigned int i = 0; i < ssa_adc_data_representation[j].size(); i++)
                packet.set(packet_ptr++, ssa_adc_data_representation[j][i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

void Comms::serialize_packet_3(std::array<char, PACKET_SIZE_BYTES>& dest) {
    std::bitset<PACKET_SIZE_BITS> packet;
    unsigned int packet_ptr = 0;
    
    debug_println("Making it here packet 3");
    encode_gyroscope_history(packet, packet_ptr);
    debug_println("Making it here");
    encode_magnetometer_history(packet, packet_ptr);
    debug_println("Making it here");
    encode_rwa_ramp_command_history(packet, packet_ptr);
    debug_println("Making it here");
    encode_mtr_command_history(packet, packet_ptr);
    debug_println("Making it here");
    encode_ssa_vec_history(packet, packet_ptr);
    debug_println("Making it here");
    encode_ssa_adc_data(packet, packet_ptr);
    debug_println("Making it here");

    //add_packet_checksum(packet, packet_ptr, &dest);
}