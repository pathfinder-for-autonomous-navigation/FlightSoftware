#ifndef GROUND_DOWNLINK_PARSER_HPP_
#define GROUND_DOWNLINK_PARSER_HPP_

#include <fsw/FCCode/MainControlLoop.hpp>
#include <vector>
#include <string>

/**
 * @brief Parses provided downlink packet into a meaningful representation
 * of the Flight Software state.
 * 
 */
class DownlinkParser {
  public:
    /**
     * @brief Construct a new Downlink Parser.
     * 
     */
    DownlinkParser(StateFieldRegistry& r,
        const std::vector<DownlinkProducer::FlowData>& flow_data);

    struct EventData {
      unsigned int ccno = 0;
      std::map<std::string, std::string> fields;
    };
    struct DownlinkData
    {
      std::map<std::string, std::string> field_data;
      std::map<std::string, EventData> event_data;

      std::string error_msg;

      std::vector<unsigned char> flow_ids;
      unsigned int cycle_no = 0;
    };

    /**
     * @brief Process a file containing a downlink, and return a JSON
     * string containing data from the most recently completed downlink
     * packet. See process_downlink_packet()'s documentation below.
     * 
     * @param filename The filepath of the file containing a downlink
     * packet.
     */
    DownlinkData process_downlink_file(const std::string& filename);
    nlohmann::json process_downlink_file_json(const std::string& filename);

  protected:
    /**
     * @brief Initialize flight software in order to get downlink flows.
     */
    MainControlLoop fcp;

    /**
     * @brief Statefield registry used to initialize main control loop and
     * check if events/fields exist in registry
     */
    StateFieldRegistry registry;

    /**
     * @brief Downlink flow data.
     */
    const std::vector<DownlinkProducer::Flow>& flow_data;

    /**
     * @brief Processes the most recent downlink packet.
     * 
     * If the packet is the start of a new downlink frame, this function
     * returns all of the state fields that were found in the most recently
     * completed downlink frame, in JSON format. Otherwise, an empty JSON
     * string is returned.
     * 
     * @param packet Character buffer containing the downlink packet.
     * @return Downlink data object.
     */
    DownlinkData process_downlink_packet(const std::vector<char>& packet);

    /**
     * @brief The most recent downlink frame that is yet incomplete and/or
     * unprocessed.
     */
    std::vector<char> most_recent_frame;
};

#endif
