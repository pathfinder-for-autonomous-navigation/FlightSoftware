/** @file apply_uplink.hpp
 * @author Tanishq Aggarwal
 * @date 12 Feb 2018
 * @brief Contains definition for the "apply uplink" function that acts on an
 * an uplink struct to modify the master state.
 */

#ifndef APPLY_UPLINK_HPP_
#define APPLY_UPLINK_HPP_

#include "uplink_struct.hpp"

namespace Comms {
    /** @brief Apply commands supplied in an uplink to the actual satellite.
     * @param uplink Struct containing uplink data to be processed.
    */
    void apply_uplink(const Uplink& uplink);
}

#endif