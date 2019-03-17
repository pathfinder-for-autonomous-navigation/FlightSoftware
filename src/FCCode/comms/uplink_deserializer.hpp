#ifndef UPLINK_DESERIALIZER_HPP_
#define UPLINK_DESERIALIZER_HPP_

#include <bitset>
#include "uplink_struct.hpp"
#include "../state/device_states.hpp"

namespace Comms {
  void deserialize_uplink(const std::bitset<Comms::UPLINK_SIZE_BITS>& mes, Uplink* uplink);
}

#endif