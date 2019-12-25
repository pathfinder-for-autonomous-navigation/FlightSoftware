#include "UplinkConsumer.h"
#include <bitstream.h>
UplinkConsumer::UplinkConsumer(StateFieldRegistry& _registry, unsigned int offset) :
    TimedControlTask<void>(_registry, "uplink_ct", offset)
{
    radio_mt_packet_len_fp = find_internal_field<size_t>("uplink.len", __FILE__, __LINE__);
    radio_mt_packet_fp = find_internal_field<char*>("uplink.ptr", __FILE__, __LINE__);

    // calcualte the maximum number of bits needed to represent the indices
    for (index_size = 1; (_registry.writable_fields.size() + 1) / (1 << index_size) > 0; ++index_size){}
    printf(debug_severity::info, "[UplinkConsumer constructor] index_size: %u", index_size);
}


void UplinkConsumer::execute()
{
    // Return if mt buffer is NULL or mt packet length is 0
    if ( !radio_mt_packet_len_fp->get() || !radio_mt_packet_fp->get())
        return;
    
    if (validate_packet())
        update_fields();

    // clear len always to avoid reprocessing bad packets
    radio_mt_packet_len_fp->set(0);
}

size_t UplinkConsumer::get_field_length(size_t field_index)
{
    if (field_index >= _registry.writable_fields.size())
        return 0;
    return _registry.writable_fields.at(field_index)->get_bit_array().size();
}

 void UplinkConsumer::update_fields()
{
    size_t packet_size = radio_mt_packet_len_fp->get()*8;
    bitstream bs (radio_mt_packet_fp->get(),  packet_size);
    size_t field_index = 0, field_len = 0, bits_consumed = 0;
    
    while (bits_consumed < packet_size)
    {
        // Get index from the bitstream
        bits_consumed += bs.nextN(index_size, reinterpret_cast<uint8_t*>(&field_index));
        if (field_index == 0) // reached end of the packet
            return;
        --field_index;
        printf(debug_severity::info, 
            "[UplinkConsumer update] Updating field: %u", field_index);
        // Get field length from the index
        field_len = get_field_length(field_index);

        auto field_p = _registry.writable_fields[field_index];
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

bool UplinkConsumer::validate_packet()
{
    size_t packet_bytes = radio_mt_packet_len_fp->get();
    bitstream bs (radio_mt_packet_fp->get(), packet_bytes);
    size_t field_index = 0, field_len = 0, bits_checked = 0, bits_consumed = 0;
    // Keep a bit map to prevent updating the same field twice
    std::vector<bool> is_field_updated(_registry.writable_fields.size(), 0);
 
    while (bits_checked < 8*radio_mt_packet_len_fp->get())
    {
        // Get index from bitstream
        bits_consumed = bs.nextN(index_size, reinterpret_cast<uint8_t*>(&field_index));

        if (field_index == 0) // reached end of the packet
            break;
        --field_index;
        // If we have already seen this field or if the number of bits consumed
        // to get the next index is not index_size
        if (is_field_updated[field_index] || bits_consumed != index_size)
        {
            printf(debug_severity::error, 
                "[UplinkConsumer validate] Field index %u (num bits: %u) already updated", 
                    field_index, bits_consumed);
            return false;
        }

        bits_checked += bits_consumed;
        is_field_updated[field_index] = true;

        // Check if index is within wri∂table_fields and get its length if it is      
        field_len = get_field_length(field_index);
        if (field_len == 0) 
        {
            printf(debug_severity::info, 
                "[UplinkConsumer validate] Invalid field_index: %u", 
                    field_index);
            return false;
        }
        
        bits_consumed = bs.seekG(field_len, bs_end);
        // Return in this case because indicates that packet is not aligned since
        // we reached the end earlier than expected
        if (bits_consumed != field_len) 
        {
            printf(debug_severity::info, 
                "[UplinkConsumer validate] Consumed %u bits but expected %u bits", 
                    bits_consumed, field_len);

            return false;
        }
        bits_checked += bits_consumed;
    }

    // Consume the padding (there should be no more than 7 bits of padding)
    uint8_t u8 = 0;
    bits_consumed = bs.nextN(8, &u8);
    
    // Return false if there were more than 8 bits left in the stream or
    // if padding was not 0
    return (u8 != 0 || bits_consumed > 7) ? false : true;
}
