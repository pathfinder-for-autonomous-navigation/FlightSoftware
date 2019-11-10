#include "DownlinkProducer.hpp"
#include <cobs.h>
#ifdef DESKTOP
    #include <cassert>
#endif

DownlinkProducer::DownlinkProducer(StateFieldRegistry& r,
    const std::vector<FlowData>& flow_data) : ControlTask<void>(r)
{
    unsigned char num_flows = flow_data.size();
    for (const FlowData& flow : flow_data) {
        Flow f(r, flow, num_flows);
        
        FlowGroup& group = flow_groups[flow.group_id];
        group.group_id = flow.group_id;
        group.flows.push_back(f);
    }

    for(auto& flow_group_it : flow_groups) {
        FlowGroup& group = flow_group_it.second;
        #ifdef DESKTOP
            assert(group.get_downlink_size() <= 70);
        #endif
    }
}

void DownlinkProducer::execute() {
    // If any flow counters are equal to their flow rate,
    // produce a packet for those flows.
    for(auto& it : flow_groups) {
        FlowGroup& flow_group = it.second;
        for(Flow& f : flow_group.flows) {
            if (f.counter == f.rate) {
                f.produce_flow_packet(nullptr);
                f.counter = 0;
            }
            f.counter++;
        }
    }
}

size_t DownlinkProducer::FlowGroup::get_downlink_size() const {
    size_t downlink_size = 0;
    downlink_size += 4; // Control cycle count
    for(const Flow& flow : flows) {
        downlink_size += flow.packet_size;
        downlink_size += 1; // Packet delimiter
    }
    if (flows.size() > 0) downlink_size -= 1; // Extra packet delimeter
    return downlink_size;
}

void DownlinkProducer::FlowGroup::produce_downlink_packet(unsigned char* dest) {
    size_t dest_it = 0; // Pointer to iterate through the size of the 
}

DownlinkProducer::Flow::Flow(
    const StateFieldRegistry& r,
    const FlowData& flow_data,
    const unsigned char _num_flows) : flow_id_sr(0, flow_data.flow_id),
                                      group_id(flow_data.group_id),
                                      flow_rate(flow_data.flow_rate),
                                      flow_counter(0)
{
    for(auto const& field_name : flow_data.field_list) {
        auto const field_ptr = r.find_readable_field(field_name);
        #ifdef DESKTOP
        assert(field_ptr);
        #endif
        field_list.push_back(field_ptr);
    }

    #ifdef DESKTOP
    assert(get_packet_size() <= 70);
    #endif
}

size_t DownlinkProducer::Flow::get_packet_size() const {
    size_t packet_size = 0;
    // Get bitcount of all fields in the flow
    size_t packet_body_bitsize = 0;
    packet_body_bitsize += flow_id_sr.bitsize();
    for(auto const& field: field_list) {
        packet_body_bitsize += field->get_bit_array().size();
    }
    size_t packet_body_bytesize = (packet_body_bitsize + 7) / 8;
    packet_size += packet_body_bytesize;

    return packet_size;
}

void DownlinkProducer::Flow::produce_flow_packet(unsigned char* packet_dest) {
    unsigned int f_it = 0; // Iterates over the packet at the bit level

    // Add flow ID to packet body
    auto const& flow_id_bits = flow_id.get_bit_array();
    flow_id_bits.to_string(packet_dest, f_it);
    f_it += flow_id_bits.size();

    // Add flow values to packet body
    for(auto const& field: field_list) {
        field->serialize();
        auto const& field_bits = field->get_bit_array();
        field_bits.to_string(packet_dest, f_it);
        f_it += field_bits.size();
    }

    // Byte-align the flow body by setting the remaining bits to zero.
    while(f_it % 8 != 0) {
        bit_array::modify_bit(packet_dest[f_it / 8], f_it, 0);
    }

    // COBS-encode the packet. The encoding will never fail since
    // the destination buffer is larger than the source buffer and
    // the destination buffer will never be null.
    cobs_encode(packet_dest, flow_packet_size, packet_dest,
                flow_packet_size + 1);
}
