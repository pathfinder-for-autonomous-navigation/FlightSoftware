#include "UplinkConsumer.h"
#include "BitStream.h"

UplinkConsumer::UplinkConsumer(StateFieldRegistry& registry, unsigned int offset) :
    TimedControlTask<void>(registry, offset),
    radio_mt_packet_len_f("uplink_consumer.mt_len"),
    radio_mt_packet_f("uplink_consumer.mt_ptr"),
    registry(registry)
{
    add_internal_field(radio_mt_packet_f);
    add_internal_field(radio_mt_packet_len_f);

    // initialize fields
    radio_mt_packet_len_f.set(0);
    radio_mt_packet_f.set(nullptr); // this must be set by QuakeManager

    for (index_size = 1; registry.writable_fields.size() / (1 << index_size) > 0; ++index_size){}
}


void UplinkConsumer::execute()
{
    // Return mt buffer is NULL or mt packet length is 0
    if ( !radio_mt_packet_len_f.get() || !radio_mt_packet_f.get())
        return;
        
    if (validate_packet())
        update_field();

    // clear len always bc we dont want to reprocess bad packets
    radio_mt_packet_len_f.set(0);
}

size_t UplinkConsumer::get_field_length(size_t field_index)
{
    if (field_index >= registry.writable_fields.size())
        return 0;
    return registry.writable_fields.at(field_index)->get_bit_array().size();
}

 void UplinkConsumer::update_field()
{
    size_t packet_bytes = (radio_mt_packet_len_f.get() + 7)/8;
    BitStream bs (radio_mt_packet_f.get(), packet_bytes);
    size_t field_index = 0, field_len = 0, bits_consumed = 0;
    while (bits_consumed < radio_mt_packet_len_f.get())
    {
        // Get index from the bitstream
        bits_consumed += bs.nextN(index_size, reinterpret_cast<uint8_t*>(&field_index));

        // Get field length from the index
        field_len = get_field_length(field_index);

        auto field_p = registry.writable_fields[field_index];
        std::vector<bool> field_bit_arr = field_p->get_bit_array();

        // Clear field's bit array
        field_bit_arr.clear();
        field_bit_arr.resize(field_len);

        // Dump into bit_array
        bits_consumed += bs.nextN(field_len, field_bit_arr);
        change_bit_arr(field_p, field_bit_arr);
    }
}

bool UplinkConsumer::validate_packet()
{
    size_t packet_bytes = (radio_mt_packet_len_f.get() + 7)/8;
    BitStream bs (radio_mt_packet_f.get(), packet_bytes);
    size_t field_index = 0, field_len = 0, bits_checked = 0, bits_seeked;
     while (bits_checked < radio_mt_packet_len_f.get())
    {
        // Get indices form bitstream
        bits_checked += bs.nextN(index_size, reinterpret_cast<uint8_t*>(&field_index));
        
        // Check if index is within writable_fields        
        field_len = get_field_length(field_index);

        if (field_len == 0) return false;

        bits_seeked = bs.seekG(field_len, bs_end);

        // Return in this case because indicates that packet is not aligned since
        // we reached the end earlier than expected
        if (bits_seeked != field_len) 
            return false;

        bits_checked += bits_seeked;
    }
    return bs.byte_offset + 1 >= bs.max_len;
}

