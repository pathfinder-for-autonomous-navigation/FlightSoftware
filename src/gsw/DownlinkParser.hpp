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

    /**
     * @brief Process a file containing a downlink, and return a JSON
     * string containing data from the most recently completed downlink
     * packet. See process_downlink_packet()'s documentation below.
     * 
     * @param filename The filepath of the file containing a downlink
     * packet.
     */
    std::string process_downlink_file(const std::string& filename);

  protected:
    /**
     * @brief Initialize flight software in order to get downlink flows.
     */
    MainControlLoop fcp;

    /**
     * @brief Downlink flow data.
     * 
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
     * 
     * @return JSON-encoded downlink data, containing two high-level keys:
     * - data: is a key-value dictionary of state field names and values.
     * - metadata: contains the
     *   - cycle count, 
     *   - an array of flow IDs in the order in which they were processed.
     *   - whether or not there were any processing errors.
     */
    std::string process_downlink_packet(const std::vector<char>& packet);

    /**
     * @brief The most recent downlink frame that is yet incomplete and/or
     * unprocessed.
     */
    std::vector<char> most_recent_frame;
};

#endif
