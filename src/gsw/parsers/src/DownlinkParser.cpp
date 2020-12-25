#include "DownlinkParser.hpp"
#include <common/Serializer.hpp>
#include <vector>
#include <fstream>
#include <json.hpp>

DownlinkParser::DownlinkParser(
    const StateFieldRegistry& r,
    const std::vector<DownlinkProducer::Flow>& f) :
    registry(r),
    flow_data(f)
{}

DownlinkParser::DownlinkData DownlinkParser::process_downlink_file(const std::string& filename) {
    std::ifstream downlink_file(filename, std::ios::in | std::ios::binary);
    if (!downlink_file.is_open()) return DownlinkData();

    std::vector<char> packet((std::istreambuf_iterator<char>(downlink_file)),
        std::istreambuf_iterator<char>());
    return process_downlink_packet(packet);
}

nlohmann::json DownlinkParser::process_downlink_file_json(const std::string& filename) {
    nlohmann::json ret;
    DownlinkData d = process_downlink_file(filename);
    if (d.flow_ids.size() != 0) to_json(ret, d);
    return ret;
}

DownlinkParser::DownlinkData DownlinkParser::process_downlink_packet(const std::vector<char>& packet) {
    // The returned value.
    DownlinkData ret;

    // If the first bit of the packet is a 1, it's the start of a new frame.
    // Otherwise the packet belongs to a previous frame.
    const bool is_first_packet_in_frame = (static_cast<unsigned char>(packet[0]) >> 7) == 0b1;
    if (!is_first_packet_in_frame || most_recent_frame.size() == 0) {
        // The downlink frame isn't done coming down yet; add the
        // packet to the downlink frame that's currently being
        // collected
        most_recent_frame.insert(most_recent_frame.end(), packet.begin(), packet.end());
    }
    else {
        // The packet is the start of a new downlink frame.
        // Process the most recently collected frame.
        const std::vector<char> frame_to_process = most_recent_frame;
        most_recent_frame = packet;

        // Process the downlink frame in four steps.

        // Step 1: Manage the downlink frame at a bit level.
        std::vector<bool> frame_bits(frame_to_process.size() * 8);
        for(size_t i = 0; i < frame_to_process.size(); i++) {
            std::bitset<8> x(frame_to_process[i]);
            for(int j = 0; j < 8; j++) {
                frame_bits[i*8 + j] = x[7 - j];
            }
        }

        // Step 2: Remove header bits from the packet.
        for(int i = frame_bits.size() / DownlinkProducer::num_bits_in_packet; i >= 0; i--) {
            frame_bits.erase(frame_bits.begin() + i * DownlinkProducer::num_bits_in_packet);
        }

        // Step 3: Process control cycle count
        unsigned int cycle_count;
        Serializer<unsigned int> cycle_count_sr;
        const std::vector<bool> cycle_count_bits(frame_bits.begin(), frame_bits.begin() + 32);
        cycle_count_sr.set_bit_array(cycle_count_bits);
        cycle_count_sr.deserialize(&cycle_count);
        ret.field_data.insert({"pan.cycle_no", std::to_string(cycle_count)});
        ret.cycle_no = cycle_count;
        frame_bits.erase(frame_bits.begin(), frame_bits.begin() + 32);

        // Step 4: Process flows by ID. If, at any point, the expected
        // size of a field exceeds the number of bits available in the
        // downlink, then stop processing.
        //
        // As flows are processed, their bits are removed from the frame.
        while(frame_bits.size() > 0) {
            // Step 4.1. Get flow ID and check if it's valid.
            unsigned char flow_id;

            Serializer<unsigned char> flow_id_sr(flow_data.size());
            if (flow_id_sr.bitsize() > frame_bits.size()) {
                // The frame doesn't contain the full flow ID. Stop processing.
                ret.error_msg = "flow ID incomplete";
                return ret;
            }
            const std::vector<bool> flow_id_bits(frame_bits.begin(),
                frame_bits.begin() + flow_id_sr.bitsize());
            flow_id_sr.set_bit_array(flow_id_bits);
            flow_id_sr.deserialize(&flow_id);
            
            if (flow_id == 0) {
                // We've reached the end of the downlink packet, since no flow
                // with ID 0 can exist.
                return ret;
            }

            // Check if flow has been repeated. This shouldn't be possible.
            if (std::find(ret.flow_ids.begin(), ret.flow_ids.end(), flow_id)
                    != ret.flow_ids.end())
            {
                ret.error_msg = "multiple flows of same ID found: " + std::to_string(flow_id);
                return ret;
            }

            // Continue processing the flow.
            ret.flow_ids.push_back(flow_id);
            frame_bits.erase(frame_bits.begin(), frame_bits.begin() + flow_id_bits.size());

            // Step 4.1.1. Find flow in Downlink Producer flows list and check if
            // it exists there.
            const DownlinkProducer::Flow* flow = nullptr;
            for(const DownlinkProducer::Flow& f : flow_data) {
                unsigned char found_flow_id;
                f.id_sr.deserialize(&found_flow_id);
                if (flow_id == found_flow_id) {
                    flow = &f;
                    break;
                }
            }
            if (!flow) {
                // Flow ID wasn't found in the list of flows. Stop processing this downlink frame.
                // If the flow ID is a zero, then the packet is over, so an error didn't occur.
                if (flow_id != 0) ret.error_msg = "flow ID invalid: " + std::to_string(flow_id);
                return ret;
            }

            for(ReadableStateFieldBase* field : flow->field_list) {
                Event* event = registry.find_event(field->name());
                if (event) {
                    // Store the original values of the control cycle count and data fields
                    unsigned int current_ccno = event->ccno->get();
                    std::vector<bit_array> field_bits_original;
                    for (ReadableStateFieldBase* data_field : event->_data_fields()) {
                        data_field->serialize();
                        field_bits_original.push_back(data_field->get_bit_array());
                    }

                    const std::vector<bool>::iterator event_end_it =
                        frame_bits.begin() + event->get_bit_array().size();
                    
                    const std::vector<bool> event_bits(frame_bits.begin(), event_end_it);
                    event->set_bit_array(event_bits);
                    
                    event->deserialize();
                    unsigned int event_ccno = event->ccno->get();

                    EventData event_data_processed;
                    event_data_processed.ccno = event_ccno;
                    for (ReadableStateFieldBase* data_field: event->_data_fields()) {
                        event_data_processed.fields.insert({data_field->name(), std::string(data_field->print())});
                    }
                    ret.event_data.insert({event->name(), event_data_processed});

                    // Reapply the original values to the control cycle count and data fields
                    event->ccno->set(current_ccno);
                    for (size_t i = 0; i < field_bits_original.size(); i++) {
                        ReadableStateFieldBase* field = event->_data_fields()[i];
                        field->set_bit_array(field_bits_original[i]);
                        field->deserialize();
                    }
                    frame_bits.erase(frame_bits.begin(), event_end_it);
                }
                else {
                    const std::vector<bool>::iterator field_end_it =
                        frame_bits.begin() + field->get_bit_array().size();

                    const std::vector<bool> field_bits(frame_bits.begin(), field_end_it);
                    field->set_bit_array(field_bits);
                    field->deserialize();

                    ret.field_data.insert({field->name(), std::string(field->print())});
                    frame_bits.erase(frame_bits.begin(), field_end_it);
                }
            }
        }
    }

    return ret;
}

nlohmann::json DownlinkParser::process_downlink_packet_json(const std::vector<char>& packet) {
    nlohmann::json ret;
    DownlinkData d = process_downlink_packet(packet);
    if (d.flow_ids.size() != 0) to_json(ret, d);
    return ret;
}

/**
 * JSON-encoded downlink data, containing two high-level keys: 
 * - data: is a key-value dictionary of state field/event names and values.
 * - metadata: contains the
 *   - cycle count, 
 *   - an array of flow IDs in the order in which they were processed.
 *   - whether or not there were any processing errors.
 */
void to_json(nlohmann::json& j, const DownlinkParser::DownlinkData& d)
{
    for(auto const& event : d.event_data)
    {
        auto const& event_name = event.first;
        auto const& event_data = event.second;

        j["data"][event_name]["control_cycle_number"] = event_data.ccno;
        for(auto const& field : event_data.fields)
        {
            j["data"][event_name]["field_data"][field.first] = field.second;
        }
    }

    for(auto const& field : d.field_data) j["data"][field.first] = field.second;

    if (d.error_msg == "") j["metadata"]["error"] = false;
    else j["metadata"]["error"] = d.error_msg;
    j["metadata"]["flow_ids"] = d.flow_ids;
    j["metadata"]["cycle_no"] = d.cycle_no;
    j["data"]["pan.cycle_no"] = std::to_string(d.cycle_no);
}
