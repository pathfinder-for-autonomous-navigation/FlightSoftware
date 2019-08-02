/**
 * @file PacketBuilder.inl
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#include "PacketBuilder.hpp"

inline PacketBuilder::PacketBuilder(QuakeMessage* pkt) : _pkt(pkt) {}

template <unsigned int bitset_size>
inline bool PacketBuilder::add_field<bitset_size>(std::bitset<bitset_size>& src) {
    if (_pkt_ptr + bitset_size > PACKET_SIZE_BITS) return false;
    // TODO implement
    return true;
}

template <typename T, unsigned int bitset_size>
inline bool PacketBuilder::add_field<T, bitset_size>(StateField<T, bitset_size>& src) {
    std::bitset<bitset_size> temp;
    src.serialize(&temp);
    add_field(temp);
}