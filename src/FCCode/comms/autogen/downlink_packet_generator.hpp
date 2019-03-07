#ifndef DOWNLINK_PACKET_GENERATOR_HPP_
#define DOWNLINK_PACKET_GENERATOR_HPP_

#include "../state/state_holder.hpp"
#include <bitset>

namespace Comms {
  constexpr unsigned int PACKET_SIZE_BITS = 560;
  constexpr unsigned int FRAME_SIZE_BITS = 5689;
  constexpr unsigned int NUM_PACKETS = (unsigned int) ceil(FRAME_SIZE_BITS / PACKET_SIZE_BITS);
  void generate_packets(std::bitset<PACKET_SIZE_BITS> (&packets)[NUM_PACKETS], unsigned int downlink_no);
}

#endif