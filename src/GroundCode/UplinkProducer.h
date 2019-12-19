#pragma once

#include "../FCCode/MainControlLoop.hpp"
#include "../FCCode/UplinkCommon.h"
#include <map>

/**
 * UplinkProducer provides operations on bitstream
 * An uplink packet is represented as a bitstream
 */
class UplinkProducer : public Uplink{
  public:

    UplinkProducer(StateFieldRegistry& r);

    /**
     * @brief Creates an UplinkPacket from given json file
     * Discards any old data in the packet and overwrites with data in filename
     * @throw runtime_error:
     *  - invalid json syntax
     *  - invalid values (tried to assign 17 to a 3 bit field)
     *  - invalid keys (tried to assign to a field that does not exist)
     *  - bitstream not large enough (bs not large enough to hold changes specified in json)
     */
    void create_from_json(bitstream& bs, const std::string& json_filename);

    /**
     * @brief Prints the UplinkPacket to STDOUT in format:
     *  index    bit width     old value --> new value   field name
     */
    void print_packet(bitstream& bs);

    /**
     * @brief Verifies that the provided bitstream is a valid uplink packet then
     * writes its contents to at the specified file
     * @throw runtime_error if bitstream is not a valid uplink packet
     */
    void to_file(const bitstream& bs, const std::string& filename);

    /**
     * @brief Creates an SBD file directly from a json file and saves it as
     * dst_file
     * @return true if successfully crreated the sbd file
     */
    bool create_sbd_from_json(const std::string& json_file, const std::string& dst_file);

    /**
     * @brief Return the maximum possible packet size
     */
    size_t get_max_possible_packet_size();

#ifndef DEBUG
  private:
#endif

    /**
     * Add an entry to the bitstream
     * @throw runtime_error if invalid index is specified
     * @return the number of bits written
     */ 
    size_t add_entry(bitstream& bs, char* val, size_t index);

    // Need this for accessing registry
    MainControlLoop fcp;

    // Maps field names to indices
    std::map<std::string, size_t> field_map;

    // (index size * number of fields) + \Sum (bit width of each field)
    size_t max_possible_packet_size;

};
