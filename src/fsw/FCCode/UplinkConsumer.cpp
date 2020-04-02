#include "UplinkConsumer.h"
#include <common/bitstream.h>

UplinkConsumer::UplinkConsumer(StateFieldRegistry& _registry, unsigned int offset) :
    TimedControlTask<void>(_registry, "uplink_ct", offset), Uplink(_registry)
{
    radio_mt_packet_len_fp = find_internal_field<size_t>("uplink.len", __FILE__, __LINE__);
    radio_mt_packet_fp = find_internal_field<char*>("uplink.ptr", __FILE__, __LINE__);
}

void UplinkConsumer::execute()
{
    if (index_size == 0)
        init_uplink();
    // Return if mt buffer is NULL or mt packet length is 0
    if ( !radio_mt_packet_len_fp->get() || !radio_mt_packet_fp->get())
        return;
    
    if (validate_packet())
        update_fields();

    // clear len always
    radio_mt_packet_len_fp->set(0);
}

 void UplinkConsumer::update_fields()
{
    if (index_size == 0)
        init_uplink();
    bitstream bs (radio_mt_packet_fp->get(), radio_mt_packet_len_fp->get());
    _update_fields(bs);
}

bool UplinkConsumer::validate_packet()
{
    if (index_size == 0)
        init_uplink();
    bitstream bs (radio_mt_packet_fp->get(), radio_mt_packet_len_fp->get());
    return _validate_packet(bs);
}
