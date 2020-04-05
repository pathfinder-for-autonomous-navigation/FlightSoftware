#include "UplinkCommon.h"

Uplink::Uplink(StateFieldRegistry& r) : registry(r), index_size(0)
{
}

void Uplink::init_uplink()
{
    // calculate the maximum number of bits needed to represent the indices
    // target variable is index_size
    for (index_size = 1; (registry.writable_fields.size() + 1) / (1 << index_size) > 0; ++index_size){}
}

bool Uplink::_validate_packet(bitstream& bs)
{
    // Start validation at beginning of bs
    bs.reset();
    size_t packet_bytes = bs.max_len;
    size_t field_index = 0, field_len = 0, bits_checked = 0, bits_consumed = 0;
    // Keep a bit map to prevent updating the same field twice
    static std::vector<bool> is_field_updated(registry.writable_fields.size(), 0);
    // Clear the bit map
    for (size_t i = 0; i < registry.writable_fields.size(); ++i)
        is_field_updated[i] = 0;
    while (bits_checked < 8*packet_bytes)
    {
        // Get index from bitstream
        bits_consumed = bs.nextN(index_size, reinterpret_cast<uint8_t*>(&field_index));

        if (field_index == 0) // reached end of the packet
            break;
        --field_index;
        // Check if index is within writable_fields and get its length if it is      
        field_len = get_field_length(field_index);
        if (field_len == 0) 
            return false;
        // If we have already seen this field or if the number of bits consumed
        // to get the next index is not index_size
        if (is_field_updated[field_index] || bits_consumed != index_size)
            return false;

        bits_checked += bits_consumed;
        is_field_updated[field_index] = true;
        
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

void Uplink::_update_fields(bitstream& bs)
{
    size_t packet_size = bs.max_len*8;
    size_t field_index = 0, field_len = 0, bits_consumed = 0;
    // Start updates at beginning of bs
    bs.reset();
    while (bits_consumed < packet_size)
    {
        // Get index from the bitstream
        bits_consumed += bs.nextN(index_size, reinterpret_cast<uint8_t*>(&field_index));
        if (field_index == 0) // reached end of the packet
            return;
        --field_index;

        // Get field length from the index
        field_len = get_field_length(field_index);
        auto field_p = registry.writable_fields[field_index];
        const std::vector<bool>& _bit_arr = field_p->get_bit_array();
        std::vector<bool>& field_bit_arr = const_cast<std::vector<bool>&>(_bit_arr);
        
        // Clear field's bit array
        for (size_t i = 0; i < field_len; ++i)
          field_bit_arr[i] = 0;

        // Dump into bit_array
        bits_consumed += bs.nextN(field_len, field_bit_arr);
        field_p->set_bit_array(field_bit_arr);
        field_p->deserialize();
    }
}

size_t Uplink::get_field_length(size_t field_index)
{
    if (field_index >= registry.writable_fields.size())
        return 0;
    return registry.writable_fields[field_index]->get_bit_array().size();
}

