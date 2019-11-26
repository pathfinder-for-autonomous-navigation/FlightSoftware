#include "UplinkConsumer.h"

UplinkConsumer::UplinkConsumer(StateFieldRegistry& registry, unsigned int offset) :
    TimedControlTask<void>(registry, offset),
    radio_mt_ready_f("uplink_consumer.mt_ready"),
    radio_mt_packet_f("uplink_consumer.mt_ptr")
{
    add_internal_field(radio_mt_packet_f);
    add_internal_field(radio_mt_ready_f);
}


void UplinkConsumer::execute()
{
    // Check radio_mt_ready_f
    if ( !radio_mt_ready_f.get() )
        return;
    
    // Check that radio_mt_packet_f is not null, hope that it's valid
    if ( radio_mt_packet_f.get() )
    {
        // complain because this is a big problem, need quakemanager to reassign
        // TODO: how to handle this? 
        return;
    }

    // parse packet

    // update statefield registry writable fields

    // Should we send an acknowledgement to the ground to say that the 
    // values have been updated? 

    // clear flag
    radio_mt_ready_f.set(0);
}