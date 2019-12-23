#include "UplinkConsumer.h"
#include <bitstream.h>
UplinkConsumer::UplinkConsumer(StateFieldRegistry& _registry, unsigned int offset) :
    TimedControlTask<void>(_registry, "uplink_ct", offset), Uplink(_registry)
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

 void UplinkConsumer::update_fields()
{
    bitstream bs (radio_mt_packet_fp->get(), radio_mt_packet_len_fp->get());
    _update_fields(bs);
}

bool UplinkConsumer::validate_packet()
{
    bitstream bs (radio_mt_packet_fp->get(), radio_mt_packet_len_fp->get());
    return _validate_packet(bs);
}
