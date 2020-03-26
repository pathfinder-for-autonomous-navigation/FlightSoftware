#include "DownlinkProducer.hpp"
#include <algorithm>
#include <set>

DownlinkProducer::DownlinkProducer(StateFieldRegistry& r,
    const unsigned int offset) : TimedControlTask<void>(r, "downlink_ct", offset),
                                 snapshot_ptr_f("downlink.ptr"),
                                 snapshot_size_bytes_f("downlink.snap_size"),
                                 shift_flows_id1_f("downlink.shift_id1", Serializer<unsigned char>(0,10,1)),
                                 shift_flows_id2_f("downlink.shift_id2", Serializer<unsigned char>(0,10,1)),
                                 toggle_flow_id_f("downlink.toggle_id", Serializer<unsigned char>(0,10,1))
{
    cycle_count_fp = find_readable_field<unsigned int>("pan.cycle_no", __FILE__, __LINE__);

    // Add snapshot fields to the registry
    add_internal_field(snapshot_ptr_f);
    add_internal_field(snapshot_size_bytes_f);

    // Add shift_flows statefield to registry and set it to default values
    add_writable_field(shift_flows_id1_f);
    add_writable_field(shift_flows_id2_f);
    shift_flows_id1_f.set(0);
    shift_flows_id2_f.set(0);

    // Add toggle command statefield to registry and set it to default of 0
    add_writable_field(toggle_flow_id_f);
    toggle_flow_id_f.set(0);
}

void DownlinkProducer::init_flows(const std::vector<FlowData>& flow_data) {
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
        flows.emplace_back(_registry, flow, num_flows); 
        if (flow.is_active) num_active_flows++;
    }

    // Set the snapshot size to the maximum possible downlink size,
    // so that the Quake Manager allocates sufficient space for
    // its copy of the snapshot.
    const size_t max_downlink_size = compute_max_downlink_size();
    snapshot = new char[max_downlink_size];
    snapshot_ptr_f.set(snapshot);
    snapshot_size_bytes_f.set(max_downlink_size);
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
    downlink_max_size_bits += (downlink_max_size_bits + num_bits_in_packet - 1) / num_bits_in_packet;

    // Byte-align downlink packet
    const size_t downlink_max_size_bytes = (downlink_max_size_bits + 7) / 8;

    return downlink_max_size_bytes;
}

size_t DownlinkProducer::compute_max_downlink_size() const {
    return compute_downlink_size(true);
}

static void add_bits_to_downlink_frame(const bit_array& field_bits,
                                       char* snapshot_ptr,
                                       size_t& packet_offset,
                                       size_t& downlink_frame_offset)
{
    const size_t field_size = field_bits.size();
    const int field_overflow = (field_size + packet_offset)
        - DownlinkProducer::num_bits_in_packet; // Number of bits in field that run past the packet end

    if(field_overflow <= 0) {
        // Contiguously write field to snapshot buffer
        field_bits.to_string(snapshot_ptr, downlink_frame_offset);
        downlink_frame_offset += field_size;
        packet_offset += field_size;
    }
    else {
        // Split field across two packets
        const int x = field_size - field_overflow; // # of bits in field that don't overrun a packet

        // Copy first part of field
        field_bits.to_string(snapshot_ptr, downlink_frame_offset, 0, x);
        downlink_frame_offset += x;
        packet_offset = 0;

        // Mark the header for a new packet
        char& packet_start = snapshot_ptr[(downlink_frame_offset / 70)];
        packet_start = bit_array::modify_bit(packet_start, 7, 0);
        downlink_frame_offset += 1;
        packet_offset += 1;

        // Copy the rest of the field
        field_bits.to_string(snapshot_ptr, downlink_frame_offset, x, field_size);
        downlink_frame_offset += field_overflow;
        packet_offset += field_overflow;
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
    size_t packet_offset = 0;         // Bit offset from the beginning
                                      // of the current downlink packet

    // Add initial packet header
    snapshot_ptr[0] = bit_array::modify_bit(snapshot_ptr[0], 7, 1);
    downlink_frame_offset += 1;
    packet_offset += 1;

    // Add control cycle count to the initial packet
    cycle_count_fp->serialize();
    const bit_array& cycle_count_bits = cycle_count_fp->get_bit_array();
    add_bits_to_downlink_frame(cycle_count_bits, snapshot_ptr, packet_offset,
            downlink_frame_offset);

    for(auto const& flow : flows) {
        if (!flow.is_active) continue;

        // Add a flow field one at a time to the snapshot, taking
        // care to add a downlink packet delimeter if the current
        // packet size exceeds 70 bytes.
        const bit_array& flow_id_bits = flow.id_sr.get_bit_array();
        add_bits_to_downlink_frame(flow_id_bits, snapshot_ptr, packet_offset,
            downlink_frame_offset);

        for(auto& field : flow.field_list) {
            Event* event = _registry.find_event(field->name());
            if (event) {
                // Event should be serialized when it is signaled
                const bit_array& event_bits = event->get_bit_array();
                add_bits_to_downlink_frame(event_bits, snapshot_ptr, packet_offset,
                    downlink_frame_offset);
            }
            else{
                field->serialize();
                const bit_array& field_bits = field->get_bit_array();
                add_bits_to_downlink_frame(field_bits, snapshot_ptr, packet_offset,
                    downlink_frame_offset);
            }
        }
    }

    // If there are bits remaining in the last character of the downlink frame,
    // fill them with zeroes.
    const unsigned int num_remaining_bits = 8 - (downlink_frame_offset % 8); 
    for(int i = num_remaining_bits - 1; i >= 0; i--) {
        char& last_char = snapshot_ptr[(downlink_frame_offset / 8)];
        last_char = bit_array::modify_bit(last_char, i, 0);
    }

    // Shift flow priorities
    if (shift_flows_id1_f.get()>0 && shift_flows_id2_f.get()>0) {
        shift_flow_priorities(shift_flows_id1_f.get(), shift_flows_id2_f.get());
        shift_flows_id1_f.set(0);
        shift_flows_id2_f.set(0);
    }

    if (toggle_flow_id_f.get()>0) {
        toggle_flow(toggle_flow_id_f.get());
        toggle_flow_id_f.set(0);
    }
}

DownlinkProducer::~DownlinkProducer() {
    delete[] snapshot;
}

#if defined GSW || defined DESKTOP
const std::vector<DownlinkProducer::Flow>& DownlinkProducer::get_flows() const {
    return flows;
}
#endif

DownlinkProducer::Flow::Flow(const StateFieldRegistry& r,
                        const FlowData& flow_data,
                        const size_t num_flows) : id_sr(num_flows),
                                                  is_active(flow_data.is_active)
{
    if (flow_data.id > num_flows || flow_data.id == 0) {
        printf(debug_severity::error, "Flow ID %d is invalid.", flow_data.id);
        assert(false);
    }

    id_sr.serialize(flow_data.id);

    for(std::string const& field_name : flow_data.field_list) {
        ReadableStateFieldBase* field_ptr = r.find_readable_field(field_name);
        Event* event_ptr = r.find_event(field_name);
        if (event_ptr && !field_ptr) {
            ReadableStateFieldBase* casted_event_ptr = dynamic_cast<ReadableStateFieldBase*>(event_ptr);
            field_list.push_back(casted_event_ptr);
        }
        else if (field_ptr && !event_ptr){
            field_list.push_back(field_ptr);
        }
        else {
            printf(debug_severity::error, 
                "Field %s was not found in registry when constructing flows.",
                field_name.c_str());
            assert(false);
        }
    }

    assert(get_packet_size() <= num_bits_in_packet - 1 - 32); // Flow should fit within one downlink packet
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
    if(id > flows.size()) {
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

void DownlinkProducer::shift_flow_priorities(unsigned char id1, unsigned char id2) {
    if(id1 > flows.size()) {
        printf(debug_severity::error, "Flow with ID %d was not found when "
                                      "trying to shift with flow ID %d.", id1, id2);
        assert(false);
    }
    if(id2 > flows.size()) {
        printf(debug_severity::error, "Flow with ID %d was not found when "
                                      "trying to shift with flow ID %d.", id2, id1);
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
    
    if (idx1>idx2) {
        for (size_t i = idx1; i > idx2; i--) {
            std::swap(flows[i], flows[i-1]);
        }
    }
    else if (idx2>idx1) {
        for (size_t i = idx1; i < idx2; i++) {
            std::swap(flows[i],flows[i+1]);
        }
    }
}