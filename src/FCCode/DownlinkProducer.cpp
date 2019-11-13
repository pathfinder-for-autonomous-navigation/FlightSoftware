#include "DownlinkProducer.hpp"
#ifdef DESKTOP
    #include <cassert>
#endif 
#include <iostream>

DownlinkProducer::DownlinkProducer(StateFieldRegistry& r,
    const unsigned int offset,
    const std::vector<FlowData>& flow_data) : TimedControlTask<void>(r, offset),
                                              snapshot_ptr_f("downlink_producer.mo_ptr"),
                                              snapshot_size_bytes_f("downlink_producer.snap_size"),
                                              flows()
{
    cycle_count_fp = find_readable_field<unsigned int>("pan.cycle_no", __FILE__, __LINE__);
    #ifdef DESKTOP
        assert(cycle_count_fp);
    #endif

    // Add snapshot fields to the registry
    add_internal_field(snapshot_ptr_f);
    add_internal_field(snapshot_size_bytes_f);

    // Create flow objects out of the flow data.
    const size_t num_flows = flow_data.size();
    for (const FlowData& flow : flow_data) {
        flows.insert({flow.id, Flow(r, flow, num_flows)});
    }

    const size_t downlink_size = compute_downlink_size();
    snapshot_size_bytes_f.set(downlink_size);
    snapshot = new char[downlink_size];
    snapshot_ptr_f.set(snapshot);
}

size_t DownlinkProducer::compute_downlink_size() const {
    size_t downlink_max_size_bits = 0;

    for (auto const& it : flows) {
        const Flow& flow = it.second;
        downlink_max_size_bits += flow.get_packet_size();
    }

    // Compute additional bits in the downlink size due to header information.
    downlink_max_size_bits += 32; // Control cycle count on first packet
    // For each 70 bytes (560 bits), we need to add a header bit for each
    // downlink packet that's a 1 or a 0.
    downlink_max_size_bits += (downlink_max_size_bits + 559) / 560;

    // Byte-align downlink packet
    const size_t downlink_max_size_bytes = (downlink_max_size_bits + 7) / 8;

    return downlink_max_size_bytes;
}

void DownlinkProducer::execute() {
    char* snapshot_ptr = snapshot_ptr_f.get();  // Pointer to output buffer
    size_t downlink_frame_offset = 0; // Bit offset from the beginning
                                      // of the snapshot buffer
    size_t packet_counter = 0;        // Current downlink packet count
    size_t packet_offset = 0;         // Bit offset from the beginning
                                      // of the current downlink packet

    // Add initial packet header
    snapshot_ptr[0] = bit_array::modify_bit(snapshot_ptr[0], 7, 1);
    downlink_frame_offset++;
    packet_offset++;

    // Add control cycle count
    const bit_array& cycle_count_ba = cycle_count_fp->get_bit_array();
    cycle_count_ba.to_string(snapshot_ptr, downlink_frame_offset);
    downlink_frame_offset += cycle_count_ba.size();
    packet_offset += cycle_count_ba.size();

    for(auto const& it : flows) {
        // Add a flow one at a time to the snapshot, taking
        // care to add a downlink packet delimeter every now
        // and then.

        const Flow& f = it.second;
        for(auto const& field : f.field_list) {
            const bit_array& field_bits = field->get_bit_array();
            const size_t field_size = field_bits.size();
            const int field_overflow = (field_size + packet_offset) - 560;
            if(field_overflow <= 0) {
                // Contiguously write field to snapshot buffer

                field_bits.to_string(snapshot_ptr, downlink_frame_offset);
                downlink_frame_offset += field_size;
                packet_offset += field_size;
            }
            else {
                // Split field across two packets

                field_bits.to_string(snapshot_ptr, downlink_frame_offset, 0, 560 - packet_offset);
                packet_counter++;
                downlink_frame_offset += 560 - packet_offset;

                char& packet_start = snapshot_ptr[(packet_counter / 70)];
                packet_start = bit_array::modify_bit(packet_start, 7, 1);
                packet_offset = 1;
                downlink_frame_offset += 1;

                field_bits.to_string(snapshot_ptr, downlink_frame_offset, 560 - packet_offset,
                    field_bits.size());
                downlink_frame_offset += field_bits.size() - (560 - packet_offset);
            }
        }
    }
}

DownlinkProducer::~DownlinkProducer() {
    delete[] snapshot;
}

DownlinkProducer::Flow::Flow(const StateFieldRegistry& r,
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
    // Get bitcount of all fields in the flow
    size_t packet_bitsize = 0;
    packet_bitsize += flow_id_sr.bitsize();

    for(auto const& field: field_list) {
        packet_bitsize += field->get_bit_array().size();
    }
    const size_t packet_bytesize = (packet_bitsize + 7) / 8;

    return packet_bytesize;
}
