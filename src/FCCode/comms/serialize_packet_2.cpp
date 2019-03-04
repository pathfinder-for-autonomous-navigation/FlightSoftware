/** @file serialize_packet_2.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for functions that serialize the second downlink packet.
 */

#include "downlink_serializer.hpp"

using namespace Comms;

static void encode_attitude_command_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        // Item: Attitude command history
        // Type: Array of vectors
        // Size per element: 29
        // Number of elements: 10
        // Description: Record of attitude commands to satellite, one every 30 seconds from the past five minutes
        while(!StateHistory::ADCS::attitude_cmd_history.empty()) {
            std::bitset<29> attitude_cmd_representation;
            trim_quaternion(StateHistory::ADCS::attitude_cmd_history.get(), &attitude_cmd_representation);
            for(unsigned int i = 0; i < attitude_cmd_representation.size(); i++)
                packet.set(packet_ptr++, attitude_cmd_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

static void encode_attitude_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        std::bitset<29> attitude_representation;
        // Item: Attitude history
        // Type: Array of vectors
        // Size per element: 29
        // Number of elements: 10
        // Description: Record of attitude of satellite, one time-averaged value over every 30 seconds from the past five minutes
        while(!StateHistory::ADCS::attitude_history.empty()) {
            trim_quaternion(StateHistory::ADCS::attitude_history.get(), &attitude_representation);
            for(unsigned int i = 0; i < attitude_representation.size(); i++)
                packet.set(packet_ptr++, attitude_representation[i]);
        }
        // Item: Recent attitude history
        // Type: Array of vectors
        // Size per element: 29
        // Number of elements: 10
        // Description: Record of attitude of satellite, one instantaneous value every second from the past 10 seconds
        while(!StateHistory::ADCS::attitude_fast_history.empty()) {
            trim_quaternion(StateHistory::ADCS::attitude_fast_history.get(), &attitude_representation);
            for(unsigned int i = 0; i < attitude_representation.size(); i++)
                packet.set(packet_ptr++, attitude_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

static void encode_rate_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        // Item: Angular rate history
        // Type: Array of vectors
        // Size per element: 30
        // Number of elements: 10
        // Description: Record of angular rate of satellite, one time-averaged value over every 30 seconds from the past five minutes
        while(!StateHistory::ADCS::rate_history.empty()) {
            std::bitset<30> rate_representation;
            trim_vector(StateHistory::ADCS::rate_history.get(), Constants::ADCS::MAX_ANGULAR_RATE, &rate_representation);
            for(unsigned int i = 0; i < rate_representation.size(); i++)
                packet.set(packet_ptr++, rate_representation[i]);
        }
        // Item: Recent angular rate history
        // Type: Array of vectors
        // Size per element: 30
        // Number of elements: 10
        // Description: Record of angular rate of satellite, one instantaneous value every second from the past 10 seconds
        while(!StateHistory::ADCS::rate_fast_history.empty()) {
            std::bitset<30> rate_representation;
            trim_vector(StateHistory::ADCS::rate_fast_history.get(), Constants::ADCS::MAX_ANGULAR_RATE, &rate_representation);
            for(unsigned int i = 0; i < rate_representation.size(); i++)
                packet.set(packet_ptr++, rate_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

static void encode_spacecraft_L_history(std::bitset<PACKET_SIZE_BITS>& packet, unsigned int& packet_ptr) {
    rwMtxRLock(&StateHistory::ADCS::adcs_state_history_lock);
        // Item: Spacecraft angular momentum history
        // Type: Array of vectors
        // Size per element: 30
        // Number of elements: 10
        // Description: Record of angular momentum of satellite as computed via the reaction wheels. One time-averaged value every 30 seconds from the past 5 minutes
        while(!StateHistory::ADCS::spacecraft_L_history.empty()) {
            std::bitset<30> L_representation;
            trim_vector(StateHistory::ADCS::spacecraft_L_history.get(), 
                Constants::ADCS::MAX_ANGULAR_RATE, &L_representation);
            for(unsigned int i = 0; i < L_representation.size(); i++)
                packet.set(packet_ptr++, L_representation[i]);
        }
        // Item: Recent spacecraft angular momentum history
        // Type: Array of vectors
        // Size per element: 30
        // Number of elements: 10
        // Description: Record of angular momentum of satellite as computed via the reaction wheels. One instantaneous value every second from the past 10 seconds
        while(!StateHistory::ADCS::spacecraft_L_fast_history.empty()) {
            std::bitset<30> L_representation;
            trim_vector(StateHistory::ADCS::spacecraft_L_fast_history.get(), 
                Constants::ADCS::MAX_ANGULAR_RATE, &L_representation);
            for(unsigned int i = 0; i < L_representation.size(); i++)
                packet.set(packet_ptr++, L_representation[i]);
        }
    rwMtxRUnlock(&StateHistory::ADCS::adcs_state_history_lock);
}

void Comms::serialize_packet_2(std::array<char, PACKET_SIZE_BYTES>& dest) {
    std::bitset<PACKET_SIZE_BITS> packet;
    unsigned int packet_ptr = 0;

    encode_attitude_command_history(packet, packet_ptr);
    encode_attitude_history(packet, packet_ptr);
    encode_rate_history(packet, packet_ptr);
    encode_spacecraft_L_history(packet, packet_ptr);

    //add_packet_checksum(packet, packet_ptr, &dest);
}
