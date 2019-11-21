#include "DownlinkProducer.hpp"
#include <algorithm>
#include <set>

DownlinkProducer::DownlinkProducer(StateFieldRegistry& r,
    const unsigned int offset,
    const std::vector<FlowData>& flow_data) : TimedControlTask<void>(r, offset),
                                              snapshot_ptr_f("downlink_producer.mo_ptr"),
                                              snapshot_size_bytes_f("downlink_producer.snap_size")
{
    cycle_count_fp = find_readable_field<unsigned int>("pan.cycle_no", __FILE__, __LINE__);
    assert(cycle_count_fp);

    // Add snapshot fields to the registry
    add_internal_field(snapshot_ptr_f);
    add_internal_field(snapshot_size_bytes_f);

    // Create flow objects out of the flow data. Ensure that
    // no two flows have the same ID.
    std::set<unsigned char> ids;

    const size_t num_flows = flow_data.size();
    flows.reserve(num_flows);
    for (const FlowData& flow : flow_data) {
        if (ids.find(flow.id) != ids.end()) {
            printf(debug_severity::error, "Two flows share the same ID: %d", flow.id);
            assert(false);
        }
        flows.emplace_back(r, flow, num_flows); 
        if (flow.is_active) num_active_flows++;
    }

    const size_t max_downlink_size = compute_max_downlink_size();
    const size_t current_downlink_size = compute_downlink_size();
    snapshot = new char[max_downlink_size];
    snapshot_ptr_f.set(snapshot);
    snapshot_size_bytes_f.set(current_downlink_size);
}

size_t DownlinkProducer::compute_downlink_size(const bool compute_max) const {
    size_t downlink_max_size_bits = 0;

    for (const Flow& flow : flows) {
        if (flow.is_active || compute_max)
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

size_t DownlinkProducer::compute_max_downlink_size() const {
    return compute_downlink_size(true);
}

static void add_field_bits_to_downlink_frame(const bit_array& field_bits,
                                             char* snapshot_ptr,
                                             size_t& packet_offset,
                                             size_t& packet_counter,
                                             size_t& downlink_frame_offset)
{
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

void DownlinkProducer::execute() {
    // Set the snapshot size in order to let the Quake Manager know about
    // the size of the current downlink.
    snapshot_size_bytes_f.set(compute_downlink_size());

    char* snapshot_ptr = snapshot_ptr_f.get();
    // Create the required iterators
    size_t downlink_frame_offset = 0; // Bit offset from the beginning
                                      // of the snapshot buffer
    size_t packet_counter = 0;        // Current downlink packet count
    size_t packet_offset = 0;         // Bit offset from the beginning
                                      // of the current downlink packet

    // Add initial packet header
    snapshot_ptr[0] = bit_array::modify_bit(snapshot_ptr[0], 7, 1);
    downlink_frame_offset++;
    packet_offset++;

    // Add control cycle count to the initial packet
    // const bit_array& cycle_count_ba = cycle_count_fp->get_bit_array();
    // cycle_count_ba.to_string(snapshot_ptr, downlink_frame_offset);
    //downlink_frame_offset += cycle_count_ba.size();
    //packet_offset += cycle_count_ba.size();

    for(auto const& flow : flows) {
        if (!flow.is_active) continue;

        // Add a flow field one at a time to the snapshot, taking
        // care to add a downlink packet delimeter if the current
        // packet size exceeds 70 bytes.
        const bit_array& flow_id_bits = flow.id_sr.get_bit_array();
        add_field_bits_to_downlink_frame(flow_id_bits, snapshot_ptr, packet_offset,
                packet_counter, downlink_frame_offset);

        for(auto const& field : flow.field_list) {
            const bit_array& field_bits = field->get_bit_array();
            add_field_bits_to_downlink_frame(field_bits, snapshot_ptr, packet_offset,
                packet_counter, downlink_frame_offset);
        }
    }
}

DownlinkProducer::~DownlinkProducer() {
    delete[] snapshot;
}

#ifdef GSW
const std::vector<DownlinkProducer::Flow>& DownlinkProducer::get_flows() const {
    return flows;
}
#endif

DownlinkProducer::Flow::Flow(const StateFieldRegistry& r,
                        const FlowData& flow_data,
                        const size_t num_flows) : id_sr(num_flows - 1),
                                                  is_active(flow_data.is_active)
{
    id_sr.serialize(flow_data.id);

    for(auto const& field_name : flow_data.field_list) {
        auto const field_ptr = r.find_readable_field(field_name);
        if(!field_ptr) {
            printf(debug_severity::error, 
                "Field %s was not found in registry when constructing flows.",
                field_name.c_str());
            assert(false);
        }
        field_list.push_back(field_ptr);
    }

    assert(get_packet_size() <= 560 - 1);
}

size_t DownlinkProducer::Flow::get_packet_size() const {
    // Get bitcount of all fields in the flow
    size_t packet_size = 0;
    packet_size += id_sr.bitsize();

    for(auto const& field: field_list) {
        packet_size += field->get_bit_array().size();
    }

    return packet_size;
}

void DownlinkProducer::toggle_flow(unsigned char id) {
    if(id >= flows.size()) {
        printf(debug_severity::error, "Flow with ID %d was not found.", id);
        assert(false);
    }

    for(size_t idx = 0; idx < flows.size(); idx++) {
        unsigned char flow_id;
        flows[idx].id_sr.deserialize(&flow_id);
        if (flow_id == id) {
            bool& is_active = flows[idx].is_active;
            if (is_active) num_active_flows--;
            else num_active_flows++;
            is_active = !is_active;
            break;
        }
    }
}

void DownlinkProducer::swap_flow_priorities(unsigned char id1, unsigned char id2) {
    if(id1 >= flows.size()) {
        printf(debug_severity::error, "Flow with ID %d was not found when "
                                      "trying to swap with flow ID %d.", id1, id2);
        assert(false);
    }
    if(id2 >= flows.size()) {
        printf(debug_severity::error, "Flow with ID %d was not found when "
                                      "trying to swap with flow ID %d.", id2, id1);
        assert(false);
    }

    size_t idx1 = 0, idx2 = 0;
    for(size_t idx = 0; idx < flows.size(); idx++) {
        unsigned char flow_id;
        flows[idx].id_sr.deserialize(&flow_id);
        if (flow_id == id1) {
            idx1 = idx;
        }
        if (flow_id == id2) {
            idx2 = idx;
        }
    }

    bool temp = flows[idx1].is_active;
    flows[idx1].is_active = flows[idx2].is_active;
    flows[idx2].is_active = temp;
    std::swap(flows[idx1], flows[idx2]);
}
