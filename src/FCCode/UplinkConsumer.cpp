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
    bitstream bs (radio_mt_packet_f.get(), radio_mt_packet_len_f.get());
    _update_fields(bs);
}

bool UplinkConsumer::validate_packet()
{
    bitstream bs (radio_mt_packet_f.get(), radio_mt_packet_len_f.get());
    return _validate_packet(bs);
}
