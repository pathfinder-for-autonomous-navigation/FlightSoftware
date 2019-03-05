/** @file uplink_deserializer.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for functions that deserialize an uplink packet.
 */

#include "uplink_deserializer.hpp"
#include "../debug.hpp"

using namespace Comms;

// TODO add battery heating option to uplink

static void deserialize_master_state(const std::bitset<UPLINK_PACKET_SIZE_BITS>& packet_bits, 
                                    unsigned int& packet_ptr, 
                                    Uplink* uplink) {
    // Master/PAN State
    unsigned int master_state_packed = (packet_bits[0] << 3) + (packet_bits[1] << 2) + (packet_bits[2] << 1) + packet_bits[3];
    packet_ptr += 3;
    if (master_state_packed > 4) {
        uplink->master_state = State::Master::MasterState::NORMAL;
        uplink->pan_state = (State::Master::PANState) (master_state_packed - 4);
    }
    else {
        uplink->master_state = (State::Master::MasterState) master_state_packed;
        switch(uplink->master_state) {
            case State::Master::MasterState::INITIALIZATION_HOLD:
                uplink->pan_state = State::Master::PANState::MASTER_INITIALIZATIONHOLD;
            break;
            case State::Master::MasterState::DETUMBLE:
                uplink->pan_state = State::Master::PANState::MASTER_DETUMBLE;
            break;
            case State::Master::MasterState::SAFE_HOLD:
                uplink->pan_state = State::Master::PANState::MASTER_SAFEHOLD;
            break;
            default:
                // Since state is not valid, leave state as-is
            break;
        }
    }
    // Is follower?
    uplink->follower_set = packet_bits[packet_ptr++];
}

static void deserialize_hat(const std::bitset<UPLINK_PACKET_SIZE_BITS>& packet_bits, 
                            unsigned int& packet_ptr, 
                            Uplink* uplink) {
    // HAT--error ignored
    for(int i = 0; i < 7; i++) uplink->hat_error_ignoreds[i] = packet_bits[packet_ptr++];
    for(int i = 0; i < 17; i++) uplink->hat_error_ignoreds_adcs[i] = packet_bits[packet_ptr++];
}

static void deserialize_adcs_info(const std::bitset<UPLINK_PACKET_SIZE_BITS>& packet_bits, 
                                    unsigned int& packet_ptr, 
                                    Uplink* uplink) {
    // ADCS State
    unsigned int adcs_state_packed = (packet_bits[packet_ptr] << 1) + packet_bits[packet_ptr+1];
    packet_ptr += 2;
    uplink->adcs_state = (State::ADCS::ADCSState) adcs_state_packed;

    // ADCS manual commanding
    uplink->control_adcs = packet_bits[packet_ptr++];
    uplink->override_propulsion_adcs_commands = packet_bits[packet_ptr++];

    if (uplink->control_adcs) {
        std::bitset<29> cmd_attitude_packed;
        for(int i = 0; i < 29; i++) cmd_attitude_packed.set(i, packet_bits[packet_ptr++]);
        expand_quaternion(cmd_attitude_packed, &(uplink->cmd_attitude));
    }
}

static void deserialize_prop_info(const std::bitset<UPLINK_PACKET_SIZE_BITS>& packet_bits, 
                                    unsigned int& packet_ptr, 
                                    Uplink* uplink) {
    uplink->override_prop_firing = packet_bits[packet_ptr++];
    if (uplink->override_prop_firing) {
        std::bitset<30> prop_firing_vector_packed;
        std::bitset<48> prop_firing_time_packed;
        for(int i = 0; i < 30; i++) prop_firing_vector_packed.set(i, packet_bits[packet_ptr++]);
        for(int i = 0; i < 48; i++) prop_firing_time_packed.set(i, packet_bits[packet_ptr++]);
    }
}

static void deserialize_resets(const std::bitset<UPLINK_PACKET_SIZE_BITS>& packet_bits, 
                                    unsigned int& packet_ptr, 
                                    Uplink* uplink) {
    uplink->power_cycle_gomspace = packet_bits[packet_ptr++];
    uplink->rewrite_gomspace_settings = packet_bits[packet_ptr++];
    uplink->power_cycle_piksi = packet_bits[packet_ptr++];
    uplink->rewrite_piksi_settings = packet_bits[packet_ptr++];
    uplink->power_cycle_quake = packet_bits[packet_ptr++];
    uplink->rewrite_quake_settings = packet_bits[packet_ptr++];
}

static void deserialize_checksum(const std::bitset<UPLINK_PACKET_SIZE_BITS>& packet_bits, 
                                const std::array<char, PACKET_SIZE_BYTES>& packet, 
                                unsigned int& packet_ptr, 
                                Uplink* uplink) {
    std::bitset<32> crc32_packed;
    for(int i = 0; i < 32; i++) crc32_packed.set(i, packet_bits[packet_ptr++]);
    packet_ptr -= 32;

    // Compute actual checksum
    std::bitset<UPLINK_PACKET_SIZE_BITS> packet_bits_copy = packet_bits;
    unsigned int packet_ptr_copy = packet_ptr;
    std::array<char, UPLINK_PACKET_SIZE_BYTES> temp_char_array;
    unsigned int expected_crc32 = 0; //add_packet_checksum(packet_bits_copy, packet_ptr_copy, &temp_char_array);

    uplink->is_crc32_valid = (expected_crc32 == crc32_packed.to_ullong());
}

void Comms::uplink_deserializer(const Devices::QLocate::Message& packet, Uplink* uplink) {
    std::bitset<UPLINK_PACKET_SIZE_BITS> packet_bits;
    for(unsigned int i = 0; i < PACKET_SIZE_BYTES; i++) {
        std::bitset<8> packet_byte(packet[i]);
        for(int j = 0; j < 8; j++)
            packet_bits.set(i * 8 + j, packet_byte[j]);
    }

    unsigned int packet_ptr = 0;
    deserialize_master_state(packet_bits, packet_ptr, uplink);
    deserialize_hat(packet_bits, packet_ptr, uplink);
    deserialize_adcs_info(packet_bits, packet_ptr, uplink);
    deserialize_prop_info(packet_bits, packet_ptr, uplink);
    deserialize_resets(packet_bits, packet_ptr, uplink);
    //deserialize_checksum(packet_bits, packet, packet_ptr, uplink);
}