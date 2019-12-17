#include "UplinkConsumer.h"
#include <bitstream.h>
UplinkConsumer::UplinkConsumer(StateFieldRegistry& registry, unsigned int offset) :
    TimedControlTask<void>(registry, "uplink_ct", offset),
    Uplink(registry),
    radio_mt_packet_len_f("uplink.len"),
    radio_mt_packet_f("uplink.ptr")
{
    add_internal_field(radio_mt_packet_f);
    add_internal_field(radio_mt_packet_len_f);

    // initialize fields
    radio_mt_packet_len_f.set(0);
    radio_mt_packet_f.set(nullptr); // this must be set by QuakeManager
}


void UplinkConsumer::execute()
{
    // Return if mt buffer is NULL or mt packet length is 0
    if ( !radio_mt_packet_len_f.get() || !radio_mt_packet_f.get())
        return;
    
    if (validate_packet())
        update_fields();

    // clear len always to avoid reprocessing bad packets
    radio_mt_packet_len_f.set(0);
}

 void UplinkConsumer::update_fields()
{
    size_t packet_size = radio_mt_packet_len_f.get()*8;
    bitstream bs (radio_mt_packet_f.get(),  packet_size);
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

bool UplinkConsumer::validate_packet()
{
    bitstream bs (radio_mt_packet_f.get(), radio_mt_packet_len_f.get());
    return _validate_packet(bs);
}
