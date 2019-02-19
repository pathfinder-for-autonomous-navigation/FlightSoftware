/** @file uplink_deserializer.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains definition for function that deserializes an uplink packet and applies
 * the content of an uplink packet to the master state.
 */

#ifndef UPLINK_DESERIALIZER_HPP_
#define UPLINK_DESERIALIZER_HPP_

#include "uplink_struct.hpp"
#include <comms_utils.hpp>

namespace Comms {
    /** @brief Deserializes an uplink packet into understandable uplink variables.
     * @param packet The packet that will be deserialized.
     * @param uplink A struct into which the packet's data will be stored.
    */
    void uplink_deserializer(const std::array<char, PACKET_SIZE_BYTES>& packet, Uplink* uplink);

    /** @brief Apply commands supplied in an uplink to the actual satellite.
     * @param uplink Struct containing uplink data to be processed.
    */
    void apply_uplink(const Uplink& uplink);
}

#endif