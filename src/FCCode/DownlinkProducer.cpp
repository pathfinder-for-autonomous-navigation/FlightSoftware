#include "DownlinkProducer.hpp"

DownlinkProducer::DownlinkProducer(StateFieldRegistry& r,
    std::vector<Flow>& _flows) : ControlTask<void>(r),
                                 flows(_flows) {}

void DownlinkProducer::execute() {
    // If any flow counters are equal to their flow rate,
    // produce a packet for those flows.
    for(unsigned char i = 0; i < flows.size(); i++) {
        Flow& f = flows[i];
        if (f.flow_counter == f.flow_rate) {
            f.produce_flow_packet();
            f.flow_counter = 0;
        }
        f.flow_counter++;
    }
}

DownlinkProducer::Flow::Flow(
    const std::vector<std::shared_ptr<ReadableStateFieldBase>>& _field_list,
    const unsigned int _flow_rate,
    const unsigned int _flow_counter) : field_list(_field_list),
                                        flow_rate(_flow_rate),
                                        flow_counter(_flow_counter)
{
    flow_packet_size = 0;
    flow_packet_size += 1; // Flow packet ID
    flow_packet_size += 4; // Control cycle # of when data was collected.

    // Get bitcount of all fields in the flow
    unsigned int flow_packet_body_bitsize;
    for(auto const& field: field_list) {
        flow_packet_body_bitsize += field->get_bitsize();
    }
    unsigned int flow_packet_body_bytesize = (flow_packet_body_bitsize + 7) / 8;
    flow_packet_size += flow_packet_body_bytesize;

    flow_packet_size = (flow_packet_size + 253) / 254; // Account for COBS encoding
}

void DownlinkProducer::Flow::produce_flow_packet() {}