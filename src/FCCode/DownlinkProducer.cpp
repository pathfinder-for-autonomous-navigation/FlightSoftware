#include "DownlinkProducer.hpp"
#ifdef DESKTOP
    #include <cassert>
#endif

DownlinkProducer::DownlinkProducer(StateFieldRegistry& r,
    const unsigned int offset,
    const std::vector<FlowData>& flow_data) : TimedControlTask<void>(r, offset),
                                              snapshot_ptr_f(),
                                              snapshot_size_bytes_f()
{
    // Add snapshot fields to the registry
    r.add_field(snapshot_ptr_f);
    r.add_field(snapshot_size_bytes_f);

    // Create flow objects out of the flow data.
    const size_t num_flows = flow_data.size();
    size_t downlink_max_size_bits = 0;
    for (const FlowData& flow : flow_data) {
        Flow f(r, flow, num_flows);
        flows[flow.id] = f;
        downlink_max_size_bits += f.get_packet_size();
    }

    // Create snapshot buffer.
    const size_t downlink_max_size_bytes = (downlink_max_size_bits + 7) / 8;
    snapshot = new char[downlink_max_size_bytes];
}

void DownlinkProducer::execute() {
    size_t downlink_frame_offset = 0;
    for(auto& it : flows) {
        // Add a flow one at a time to the snapshot, taking
        // care to add a downlink packet delimeter every now
        // and then.
    }
}

DownlinkProducer::~DownlinkProducer() {
    delete[] snapshot;
}

DownlinkProducer::Flow::Flow(
    const StateFieldRegistry& r,
    const FlowData& flow_data,
    const size_t num_flows) : flow_id_sr(0, num_flows)
{
    flow_id_sr.serialize(flow_data.id);

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

void DownlinkProducer::Flow::produce_flow_packet(unsigned char* packet_dest,
                                                 size_t offset) {
    // Add flow ID to packet body
    auto const& flow_id_bits = flow_id_sr.get_bit_array();
    flow_id_bits.to_string(packet_dest, offset);
    offset += flow_id_bits.size();

    // Add flow values to packet body
    for(auto const& field: field_list) {
        field->serialize();
        auto const& field_bits = field->get_bit_array();
        field_bits.to_string(packet_dest, offset);
        offset += field_bits.size();
    }
}
