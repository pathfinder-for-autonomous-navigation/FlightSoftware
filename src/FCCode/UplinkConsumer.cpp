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
    // Return if either radio_mt_packet_len_f is 0 or mt_packet is null
    if ( !radio_mt_packet_len_f.get() || !radio_mt_packet_f.get())
        return;
        
    if ( !validate_packet())
        return;

    // parse packet

    // update statefield registry writable fields

    // Should we send an acknowledgement to the ground to say that the 
    // values have been updated? 

    // clear length
    radio_mt_packet_len_f.set(0);
}

size_t UplinkConsumer::get_field_length(size_t field_index)
{
    if (field_index >= registry.writable_fields.size())
        return 0;
    return registry.writable_fields.at(field_index)->get_bit_array().size();
}

 void UplinkConsumer::update_field(size_t field_index)
{
    BitStream bs (radio_mt_packet_f.get(), radio_mt_packet_len_f.get());
    size_t field_len = get_field_length(field_index);
    if (field_len < 0) return;

    auto field_p = registry.writable_fields[field_index];
    auto field_bit_arr = field_p->get_bit_array();

    bs >> field_bit_arr;
    change_bit_arr(field_p, field_bit_arr);
}

bool UplinkConsumer::validate_packet()
{
    BitStream bs (radio_mt_packet_f.get(), radio_mt_packet_len_f.get());
    return false;
}

