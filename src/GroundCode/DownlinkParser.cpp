#include "DownlinkParser.hpp"
#include <Serializer.hpp>
#include <vector>
#include <fstream>
#include <json.hpp>

DownlinkParser::DownlinkParser(StateFieldRegistry& r,
                               const std::vector<DownlinkProducer::FlowData>& flow_metadata) :
    fcp(r, flow_metadata),
    flow_data(fcp.get_downlink_producer()->get_flows()) {}

std::string DownlinkParser::process_downlink_file(const std::string& filename) {
    std::ifstream downlink_file(filename, std::ios::in | std::ios::binary);
    if (!downlink_file.is_open()) return "File not found.\n";

    std::vector<char> packet((std::istreambuf_iterator<char>(downlink_file)),
        std::istreambuf_iterator<char>());
    return process_downlink_packet(packet);
}

std::string DownlinkParser::process_downlink_packet(const std::vector<char>& packet) {
    // The returned JSON object.
    nlohmann::json ret;

    // If the first bit of the packet is a 1, it's the start of a new frame.
    // Otherwise the packet belongs to a previous frame.
    const bool is_first_packet_in_frame = packet[0] >> 7; 
    if (!is_first_packet_in_frame) {
        // The downlink frame isn't done coming down yet; add the
        // packet to the frame
        most_recent_frame.reserve(most_recent_frame.size() + packet.size());
        most_recent_frame.insert(most_recent_frame.end(), packet.begin(), packet.end());
    }
    else {
        // The packet is the start of a new downlink frame.
        std::vector<char> old_frame = most_recent_frame;
        most_recent_frame = packet;

        // Process the old downlink frame in four steps:

        // Step 1: Manage the old downlink frame at a bit level.
        std::vector<bool> frame_bits(old_frame.size() * 8);
        for(size_t i = 0; i < frame_bits.size(); i++) {
            frame_bits[i] = old_frame[i / 8] >> (7 - (i % 8));
        }

        // Step 2: remove headers from the packet.
        for(int i = frame_bits.size() / 560; i >= 0; i--) {
            frame_bits.erase(frame_bits.begin() + i * 560);
        }

        // Step 3: Process control cycle count
        unsigned int cycle_count;
        Serializer<unsigned int> cycle_count_sr;
        std::vector<bool> cycle_count_bits(frame_bits.begin(), frame_bits.begin() + 32);
        cycle_count_sr.set_bit_array(cycle_count_bits);
        cycle_count_sr.deserialize(&cycle_count);
        ret["pan.cycle_no"]["value"] = cycle_count;
        ret["pan.cycle_no"]["num_bits"] = 32;
        frame_bits.erase(frame_bits.begin(), frame_bits.begin() + 32);

        // Step 4: Process flows by ID. If, at any point, the expected
        // size of a field exceeds the number of bits available in the
        // downlink, then stop processing.
        while(frame_bits.size() > 0) {
            // Step 4.1. Get flow ID.
            Serializer<unsigned char> flow_id_sr(flow_data.size());
            unsigned char flow_id;
            const std::vector<bool>::iterator flow_id_end_it =
                    frame_bits.begin() + flow_id_sr.bitsize();
            if (flow_id_end_it > frame_bits.begin() + frame_bits.size()) {
                return ret.dump();
            }
            std::vector<bool> flow_id_bits(frame_bits.begin(), flow_id_end_it);
            flow_id_sr.set_bit_array(flow_id_bits);
            flow_id_sr.deserialize(&flow_id);
            frame_bits.erase(frame_bits.begin(), frame_bits.begin() + flow_id_bits.size());

            // Step 4.2. Find flow in Downlink Producer flows list. 
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
                // The flow wasn't found. Stop processing this downlink frame.
                return ret.dump();
            }

            // Step 4.3. Process the items in the flow.
            for(ReadableStateFieldBase* field : flow->field_list) {
                const std::vector<bool>::iterator field_end_it =
                    frame_bits.begin() + field->get_bit_array().size();
                if (field_end_it > frame_bits.begin() + frame_bits.size()) {
                    return ret.dump();
                }

                std::vector<bool> field_bits(frame_bits.begin(), field_end_it);
                field->set_bit_array(field_bits);
                field->deserialize();
                ret[field->name()]["value"] = std::string(field->print());
                ret[field->name()]["num_bits"] = field_bits.size();

                frame_bits.erase(frame_bits.begin(), field_end_it);
            }
        }
    }

    return ret.dump();
}
