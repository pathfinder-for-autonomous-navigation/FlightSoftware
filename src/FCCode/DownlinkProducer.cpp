#include "DownlinkProducer.hpp"
#include <cobs.h>
#ifdef DESKTOP
    #include <cassert>
#endif

DownlinkProducer::DownlinkProducer(StateFieldRegistry& r,
    const std::vector<FlowData>& flow_data) : ControlTask<void>(r)
{
    unsigned char num_flows = flow_data.size();
    for (auto const& flow : flow_data) {
        Flow f(r, flow, num_flows);
        flows.push_back(f);
    }
}

void DownlinkProducer::execute() {
    // If any flow counters are equal to their flow rate,
    // produce a packet for those flows.
    for(size_t i = 0; i < flows.size(); i++) {
        Flow& f = flows[i];
        if (f.flow_counter == f.flow_rate) {
            f.produce_flow_packet(nullptr);
            f.flow_counter = 0;
        }
        f.flow_counter++;
    }
}

DownlinkProducer::Flow::Flow(
    const StateFieldRegistry& r,
    const FlowData& flow_data,
    const unsigned char _num_flows) : flow_id(0, flow_data.flow_id),
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

    flow_packet_size += 4; // Control cycle # of when data was collected.

    // Get bitcount of all fields in the flow
    unsigned int flow_packet_body_bitsize;
    flow_packet_body_bitsize += flow_id.bitsize();
    for(auto const& field: field_list) {
        flow_packet_body_bitsize += field->get_bit_array().size();
    }
    unsigned int flow_packet_body_bytesize = (flow_packet_body_bitsize + 7) / 8;
    flow_packet_size += flow_packet_body_bytesize;

    #ifdef DESKTOP
    assert(flow_packet_size <= 70);
    #endif
}

int DownlinkProducer::Flow::produce_flow_packet(char* packet_dest) {
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

    // Add control cycle count to flow packet
    f_it = f_it / 8; // Iterate over the byte level now
    unsigned int control_cycle_no = 2;
    memcpy(packet_dest + f_it, (char*)(&control_cycle_no), sizeof(unsigned int));

    // COBS-encode the packet
    auto cobs_encode_result = cobs_encode(packet_dest, flow_packet_size,
                                packet_dest, flow_packet_size + 1);
    if (cobs_encode_result.status == COBSR_ENCODE_OK) {
        return cobs_encode_result.out_len;
    }
    else if (cobs_encode_result.status == COBSR_ENCODE_OUT_BUFFER_OVERFLOW) {
        return -1;
    }
    else if (cobs_encode_result.status == COBSR_ENCODE_NULL_POINTER) {
        return -2;
    }
}
