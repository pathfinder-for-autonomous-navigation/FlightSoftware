#include "DownlinkParser.hpp"
#include <common/Serializer.hpp>
#include <vector>
#include <fstream>
#include <json.hpp>

DownlinkParser::DownlinkParser(StateFieldRegistry& r,
                               const std::vector<DownlinkProducer::FlowData>& flow_data) :
    fcp(r, flow_data),
    registry(r),
    flow_data(fcp.get_downlink_producer()->get_flows()),
    packet_num(0) {}

std::string DownlinkParser::process_downlink_file(const std::string& filename) {
    std::ifstream downlink_file(filename, std::ios::in | std::ios::binary);
    if (!downlink_file.is_open()) return "Error: file not found.";

    std::vector<char> packet((std::istreambuf_iterator<char>(downlink_file)),
        std::istreambuf_iterator<char>());
    return process_downlink_packet(packet);
}

bool DownlinkParser::check_is_first_packet(const std::vector<char>& packet, nlohmann::json& ret){
    // packet is a downlink packet to check, ret is the json object to modify with debugging data if required.

    std::string log_str;
    using json = nlohmann::json;
    ret["metadata"]["check_flow_ids"] = json::array();

    // Process the downlink frame in four steps.

    // Step 1: Reverse the bits per byte in the packet.
    std::vector<bool> frame_bits(packet.size() * 8);
    for(size_t i = 0; i < packet.size(); i++) {
        std::bitset<8> x(packet[i]);
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
    // ret["data"]["pan.cycle_no"] = std::to_string(cycle_count);
    // ret["metadata"]["cycle_no"] = cycle_count;
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
            log_str += "Flow ID incomplete.";
            ret["metadata"]["check_log"] = log_str;

            // We hit the end of the packet, so we can't process any more flows.
            break;
        }
        const std::vector<bool> flow_id_bits(frame_bits.begin(),
            frame_bits.begin() + flow_id_sr.bitsize());
        flow_id_sr.set_bit_array(flow_id_bits);
        flow_id_sr.deserialize(&flow_id);
        
        if (flow_id == 0) {
            // We've reached the end of the downlink packet, since no flow
            // with ID 0 can exist.
            break;
        }

        // Check if flow has been repeated. This shouldn't be possible.
        const std::vector<unsigned char> found_flow_ids = ret["metadata"]["check_flow_ids"];
        if (std::find(found_flow_ids.begin(), found_flow_ids.end(), flow_id)
                != found_flow_ids.end())
        {
            log_str += "multiple flows of same ID found: " + std::to_string(flow_id);
            ret["metadata"]["check_log"] = log_str;
            break;
        }

        // Continue processing the flow.
        ret["metadata"]["check_flow_ids"].push_back(flow_id); // Log the newly found flow ID.
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
            if (flow_id != 0) ret["metadata"]["error"] = "flow ID invalid: " + std::to_string(flow_id);
            break;
        }

        /**
         * Step 4.2. Process the items in the flow, and add the items
         * to the downlink data.
         * Field information will be stored like so: 
         * "data": {
         *      "event_name": {
         *          "control_cycle_number": event control cycle number,
         *          "field_data": {
         *              "field1_name": field1 value,
         *              "field2_name": field2 value,
         *              "field3_name": field3 value
         *          }
         *      },
         *      "readable_field_name": readable field value
         * }
         */
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
                // unsigned int event_ccno = event->ccno->get();

                // ret["data"][event->name()]["control_cycle_number"] = event_ccno;
                // for (ReadableStateFieldBase* data_field: event->_data_fields()) {
                //     ret["data"][event->name()]["field_data"][data_field->name()] = std::string(data_field->print());
                // }

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

                // ret["data"][field->name()] = std::string(field->print());
                frame_bits.erase(frame_bits.begin(), field_end_it);
            }
        }
    }

    const std::vector<unsigned char> first_packet_flow_ids{1,2,7,8,13,14}; // default first packet flows
    const std::vector<unsigned char> second_packet_flow_ids{15,17,18,19,25,26}; // default second packet flows
    const std::vector<unsigned char> critical_first_packet_flow_ids{1,2}; // flows that must always be enabled in the first packet.

    // const std::vector<unsigned char> all_flow_ids{1,2,7,8,13,14,15,17,18,19,25,26};

    const std::vector<unsigned char> ret_flow_ids = ret["metadata"]["check_flow_ids"];

    bool is_first_packet = true;
    for(unsigned char critical_flow_id : critical_first_packet_flow_ids){
        bool found_in_flow_ids = ret_flow_ids.end() != find(ret_flow_ids.begin(), ret_flow_ids.end(), critical_flow_id);
        is_first_packet = is_first_packet && found_in_flow_ids;
    }
    ret["metadata"]["is_first_packet"] = std::to_string(is_first_packet);

    // const bool matching = (first_packet_flow_ids == ret_flow_ids);
    const bool matching = is_first_packet;

    return matching;
}

std::string DownlinkParser::process_downlink_packet(const std::vector<char>& packet) {
    // The returned JSON object.
    using json = nlohmann::json;
    json ret;

    // If the first bit of the packet is a 1, it's the start of a new frame.
    // Otherwise the packet belongs to a previous frame.
    // const bool is_first_packet_in_frame = (static_cast<unsigned char>(packet[0]) >> 7) == 0b1;

    const bool is_first_packet_in_frame = DownlinkParser::check_is_first_packet(packet, ret);
    // return ret.dump();


    // if is first_packet in frame, clear most_recent_frame    
    // insert
    // parse always from most_recent_frame
    std::string log_str;

    if (is_first_packet_in_frame) {
        // we have a first packet in frame, clear the buffer
        most_recent_frame.clear();
        log_str += "Start of new frame!";
        ret["metadata"]["log"] = log_str;

        packet_num = 0;
    }
    
    most_recent_frame.insert(most_recent_frame.end(), packet.begin(), packet.end());
    packet_num = packet_num + 1;

    ret["metadata"]["packet_num"] = std::to_string(packet_num);

    // The packet is the start of a new downlink frame.
    // Process the most recently collected frame.
    const std::vector<char> frame_to_process = most_recent_frame;


    ret["metadata"]["error"] = false;
    ret["metadata"]["flow_ids"] = json::array();

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
    ret["data"]["pan.cycle_no"] = std::to_string(cycle_count);
    ret["metadata"]["cycle_no"] = cycle_count;
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
            ret["metadata"]["error"] = "flow ID incomplete";
            return ret.dump();
        }
        const std::vector<bool> flow_id_bits(frame_bits.begin(),
            frame_bits.begin() + flow_id_sr.bitsize());
        flow_id_sr.set_bit_array(flow_id_bits);
        flow_id_sr.deserialize(&flow_id);
        
        if (flow_id == 0) {
            // We've reached the end of the downlink packet, since no flow
            // with ID 0 can exist.
            return ret.dump();
        }

        // Check if flow has been repeated. This shouldn't be possible.
        const std::vector<unsigned char> found_flow_ids = ret["metadata"]["flow_ids"];
        if (std::find(found_flow_ids.begin(), found_flow_ids.end(), flow_id)
                != found_flow_ids.end())
        {
            ret["metadata"]["error"] = "multiple flows of same ID found: " + std::to_string(flow_id);
            return ret.dump();
        }

        // Continue processing the flow.
        ret["metadata"]["flow_ids"].push_back(flow_id);
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
            if (flow_id != 0) ret["metadata"]["error"] = "flow ID invalid: " + std::to_string(flow_id);
            return ret.dump();
        }

        /**
         * Step 4.2. Process the items in the flow, and add the items
         * to the downlink data.
         * Field information will be stored like so: 
         * "data": {
         *      "event_name": {
         *          "control_cycle_number": event control cycle number,
         *          "field_data": {
         *              "field1_name": field1 value,
         *              "field2_name": field2 value,
         *              "field3_name": field3 value
         *          }
         *      },
         *      "readable_field_name": readable field value
         * }
         */
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

                ret["data"][event->name()]["control_cycle_number"] = event_ccno;
                for (ReadableStateFieldBase* data_field: event->_data_fields()) {
                    ret["data"][event->name()]["field_data"][data_field->name()] = std::string(data_field->print());
                }

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

                ret["data"][field->name()] = std::string(field->print());
                frame_bits.erase(frame_bits.begin(), field_end_it);
            }
        }
    }
    

    return ret.dump();
}
