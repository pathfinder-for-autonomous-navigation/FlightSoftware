/** @file downlink_serializer.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains definitions for functions that perform packet downlink serialization.
 */

#ifndef DOWNLINK_SERIALIZER_HPP_
#define DOWNLINK_SERIALIZER_HPP_

#include "../state/state_holder.hpp"
#include "../state/state_history_holder.hpp"
#include "../controllers/constants.hpp"
#include <comms_utils.hpp>

namespace Comms {
    /** @brief Serialize the first packet for downlink.
     *  @param dest The char array into which the serialized packet should be stored.
     * */
    void serialize_packet_1(std::array<char, Comms::PACKET_SIZE_BYTES>& dest);

    /** @brief Serialize the second packet for downlink.
     *  @param dest The char array into which the serialized packet should be stored.
     * */
    void serialize_packet_2(std::array<char, Comms::PACKET_SIZE_BYTES>& dest);

    /** @brief Serialize the third packet for downlink.
     *  @param dest The char array into which the serialized packet should be stored.
     * */
    void serialize_packet_3(std::array<char, Comms::PACKET_SIZE_BYTES>& dest);
    
    /** @brief Serialize all three packets for downlink.
     *  @param dest The 2D char array into which the serialized packets should be stored.
     * */
    inline void downlink_serializer(std::array<char, Comms::PACKET_SIZE_BYTES> (&dest)[3]) {
        serialize_packet_1(dest[0]);
        serialize_packet_2(dest[1]);
        serialize_packet_3(dest[2]);
    }
}

#endif