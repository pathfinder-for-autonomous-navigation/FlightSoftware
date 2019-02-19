/** @file serialize_packet_2.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for functions that serialize the second downlink packet.
 */

#include "downlink_serializer.hpp"

using namespace Comms;

static void encode_attitude_command_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        while(!StateHistory::ADCS::attitude_cmd_history.empty()) {
            std::bitset<29> attitude_cmd_representation;
            trim_quaternion(StateHistory::ADCS::attitude_cmd_history.get(), &attitude_cmd_representation);
            for(unsigned int i = 0; i < attitude_cmd_representation.size(); i++ && packet_ptr--)
                packet.set(packet_ptr, attitude_cmd_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

static void encode_attitude_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        while(!StateHistory::ADCS::attitude_history.empty()) {
            std::bitset<29> attitude_representation;
            trim_quaternion(StateHistory::ADCS::attitude_history.get(), &attitude_representation);
            for(unsigned int i = 0; i < attitude_representation.size(); i++ && packet_ptr--)
                packet.set(packet_ptr, attitude_representation[i]);
        }
        while(!StateHistory::ADCS::attitude_fast_history.empty()) {
            debug_println("getting stuck here how many times?");
            std::bitset<29> attitude_representation;
            trim_quaternion(StateHistory::ADCS::attitude_fast_history.get(), &attitude_representation);
            for(unsigned int i = 0; i < attitude_representation.size(); i++ && packet_ptr--)
                packet.set(packet_ptr, attitude_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

static void encode_rate_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        while(!StateHistory::ADCS::rate_history.empty()) {
            std::bitset<30> rate_representation;
            trim_vector(StateHistory::ADCS::rate_history.get(), 0, 5, &rate_representation); // TODO check numbers
            for(unsigned int i = 0; i < rate_representation.size(); i++ && packet_ptr--)
                packet.set(packet_ptr, rate_representation[i]);
        }
        while(!StateHistory::ADCS::rate_fast_history.empty()) {
            std::bitset<30> rate_representation;
            trim_vector(StateHistory::ADCS::rate_fast_history.get(), 0, 5, &rate_representation); // TODO check numbers
            for(unsigned int i = 0; i < rate_representation.size(); i++ && packet_ptr--)
                packet.set(packet_ptr, rate_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

static void encode_spacecraft_L_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        while(!StateHistory::ADCS::spacecraft_L_history.empty()) {
            std::bitset<30> L_representation;
            trim_vector(StateHistory::ADCS::spacecraft_L_history.get(), 0, 5, &L_representation); // TODO check numbers
            for(unsigned int i = 0; i < L_representation.size(); i++ && packet_ptr--)
                packet.set(packet_ptr, L_representation[i]);
        }
        while(!StateHistory::ADCS::spacecraft_L_fast_history.empty()) {
            std::bitset<30> L_representation;
            trim_vector(StateHistory::ADCS::spacecraft_L_fast_history.get(), 0, 5, &L_representation); // TODO check numbers
            for(unsigned int i = 0; i < L_representation.size(); i++ && packet_ptr--)
                packet.set(packet_ptr, L_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

void Comms::serialize_packet_2(std::array<char, PACKET_SIZE_BYTES>& dest) {
    std::bitset<PACKET_SIZE_BITS> packet;
    unsigned int packet_ptr = 0;
    
    debug_println("Making it here 2");
    encode_attitude_command_history(packet, packet_ptr);
    debug_println("Making it here 2");
    encode_attitude_history(packet, packet_ptr);
    debug_println("Making it here 2");
    encode_rate_history(packet, packet_ptr);
    debug_println("Making it here 2");
    encode_spacecraft_L_history(packet, packet_ptr);
    debug_println("Making it here 2");

    //add_packet_checksum(packet, packet_ptr, &dest);
}
