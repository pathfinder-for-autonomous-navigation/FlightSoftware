/** @file apply_uplink.hpp
 * @author Tanishq Aggarwal
 * @date 12 Feb 2018
 * @brief Contains implementation for the "apply uplink" function that acts on an
 * an uplink struct to modify the master state.
 */

#include "apply_uplink.hpp"

void Comms::apply_uplink(const Comms::Uplink& uplink) {
    if (!uplink.is_crc32_valid) return;
    // TODO
}