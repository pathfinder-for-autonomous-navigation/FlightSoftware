#include "UplinkCommon.h"

Uplink::Uplink(StateFieldRegistry& r) : registry(r)
{
    // calculate the maximum number of bits needed to represent the indices
    for (index_size = 1; (registry.writable_fields.size() + 1) / (1 << index_size) > 0; ++index_size){}
}

bool Uplink::_validate_packet(bitstream& bs)
{
    size_t packet_bytes = bs.max_len;
  
    size_t field_index = 0, field_len = 0, bits_checked = 0, bits_consumed = 0;
    // Keep a bit map to prevent updating the same field twice
    std::vector<bool> is_field_updated(registry.writable_fields.size(), 0);
    while (bits_checked < 8*packet_bytes)
    {
        // Get index from bitstream
        bits_consumed = bs.nextN(index_size, reinterpret_cast<uint8_t*>(&field_index));

        if (field_index == 0) // reached end of the packet
            break;
        --field_index;
        // If we have already seen this field or if the number of bits consumed
        // to get the next index is not index_size
        if (is_field_updated[field_index] || bits_consumed != index_size)
            return false;

        bits_checked += bits_consumed;
        is_field_updated[field_index] = true;

        // Check if index is within wri∂table_fields and get its length if it is      
        field_len = get_field_length(field_index);
        if (field_len == 0) 
            return false;
        
        bits_consumed = bs.seekG(field_len, bs_end);
        // Return in this case because indicates that packet is not aligned since
        // we reached the end earlier than expected
        if (bits_consumed != field_len) 
            return false;

        bits_checked += bits_consumed;
    }

    // Consume the padding (there should be no more than 7 bits of padding)
    uint8_t u8 = 0;
    bits_consumed = bs.nextN(8, &u8);
    
    // Return false if there were more than 8 bits left in the stream or
    // if padding was not 0
    return (u8 != 0 || bits_consumed > 7) ? false : true;
}

size_t Uplink::get_field_length(size_t field_index)
{
    if (field_index >= registry.writable_fields.size())
        return 0;
    return registry.writable_fields.at(field_index)->get_bit_array().size();
}
