/**
 * @file PacketBuilder.hpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#ifndef PACKET_BUILDER_HPP_
#define PACKET_BUILDER_HPP_

#include <QLocate.hpp>
#include <StateField.hpp>
#include <bitset>

/**
 * @brief Creates a downlink packet by sequentially adding bitsets to a
 * fixed-size array until no more can be added.
 */
class PacketBuilder {
private:
  unsigned int _pkt_ptr;
  QuakeMessage *_pkt;

public:
  static constexpr unsigned int PACKET_SIZE_BYTES = 70;
  static constexpr unsigned int PACKET_SIZE_BITS = 560;

  PacketBuilder(QuakeMessage *pkt);

  template <unsigned int bitset_size>
  bool add_field(std::bitset<bitset_size> &src);

  template <typename T, unsigned int bitset_size>
  bool add_field(StateField<T, bitset_size> &src);
};

#endif