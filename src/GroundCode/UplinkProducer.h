#pragma once

#include "../FCCode/MainControlLoop.hpp"
#include <bitstream.h>
#include <vector>
#include <string>
#include <map>
#include <exception>


/**
 * Uplink Packet Data Structure
 */
struct UplinkPacket {
  inline UplinkPacket(char* data, size_t data_size) : bs(data, data_size){}
  bitstream bs;
};

/**
 * UplinkProducer provides operations on an UplinkPacket
 */
class UplinkProducer {
  public:

    UplinkProducer(StateFieldRegistry& r);

    /**
     * Creates an UplinkPacket from given json file
     * Discards any old data in the packet and overwrites with data in filename
     */
    void create_from_json(UplinkPacket& up, const std::string& filename);

    size_t add_entry(UplinkPacket& up, char* val, size_t index);

    size_t get_field_length(size_t field_index);

    /**
     * Prints the UplinkPacket in format:
     * [Index][Data] (field name) (field size)
     */
    void to_string(const UplinkPacket& up);

    /**
     * Verifies then saves the UplinkPacket in the file
     */
    void to_file(const UplinkPacket& up, const std::string& filename);

  private:
    StateFieldRegistry& registry;
    MainControlLoop fcp;
    // maps field names to indices
    std::map<std::string, size_t> field_map;
    // maximum number of bits to represent a field
    size_t index_size;
};
