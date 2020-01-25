#pragma once

#include <fsw/FCCode/MainControlLoop.hpp>
#include <fsw/FCCode/UplinkCommon.h>
#include <map>

/**
 * UplinkProducer provides operations on a bitstream. A bitstream represents
 * an uplink packet
 * 
 * Dependencies: Uplink
 * 
 */
class UplinkProducer : public Uplink{
  public:

    UplinkProducer(StateFieldRegistry& r);

    /**
     * Creates an UplinkPacket from given json file
     * Discards any old data in the packet and overwrites with data in filename
     * @throw runtime_error if invalid json format
     */
    void create_from_json(bitstream& bs, const std::string& filename);

    /**
     * Prints the UplinkPacket to STDOUT
     */
    void print_packet(bitstream& bs);

    /**
     * Verifies then saves the UplinkPacket in the file
     * @throw runtime_error if bitstream is not a valid uplink packet
     */
    void to_file(const bitstream& bs, const std::string& filename);

    /**
     * Creates an SBD file directly from a json file
     * @return true if successful
     */
    bool create_sbd_from_json(const std::string& json_file, const std::string& dst_file);

    /**
     * Return the maximum possible packet size
     */
    const size_t get_max_possible_packet_size();
#ifndef DEBUG
  private:
#endif

    /**
     * Add an entry to the bitstream
     * @throw runtime_error if invalid index is specified
     * @return the number of bits written
     */ 
    size_t add_entry(bitstream& bs, char* val, size_t index);

    MainControlLoop fcp;

    // maps field names to indices
    std::map<std::string, size_t> field_map;

    size_t max_possible_packet_size;

};
