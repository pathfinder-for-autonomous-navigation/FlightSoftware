#include "UplinkConsumer.h"
#include <common/bitstream.h>
#include <common/debug_console.hpp>

UplinkConsumer::UplinkConsumer(StateFieldRegistry& _registry) :
    TimedControlTask<void>(_registry, "uplink_ct"), Uplink(_registry)
{
    radio_mt_packet_len_fp = find_internal_field<size_t>("uplink.len", __FILE__, __LINE__);
    radio_mt_packet_fp = find_internal_field<char*>("uplink.ptr", __FILE__, __LINE__);
}

void UplinkConsumer::execute()
{
    debug_console::printf(debug_severity::error, "UplinkConsumer::execute()\n");

    if (index_size == 0){
        init_uplink();
        debug_console::printf(debug_severity::error, "Post Init Uplink\n");
    }
    // Return if mt buffer is NULL or mt packet length is 0
    if ( !radio_mt_packet_len_fp->get() || !radio_mt_packet_fp->get()){
        debug_console::printf(debug_severity::error, "Null or length 0\n");
        return;
    }
    
    if (validate_packet())
        update_fields();

    // clear len always
    radio_mt_packet_len_fp->set(0);
}

 void UplinkConsumer::update_fields()
{
    debug_console::printf(debug_severity::error, "Public updating fields\n");

    if (index_size == 0)
        init_uplink();
    bitstream bs (radio_mt_packet_fp->get(), radio_mt_packet_len_fp->get());
    _update_fields(bs);
    debug_console::printf(debug_severity::error, "Post update fields\n");

}

bool UplinkConsumer::validate_packet()
{
    debug_console::printf(debug_severity::error, "Validating\n");

    if (index_size == 0)
        init_uplink();
    bitstream bs (radio_mt_packet_fp->get(), radio_mt_packet_len_fp->get());
    bool validate_packet_result = _validate_packet(bs);
    debug_console::printf(debug_severity::error, "Validate result %d\n", validate_packet_result);

    return _validate_packet(bs);
}
