#pragma once

#include "../FCCode/MainControlLoop.hpp"
#include "../FCCode/UplinkCommon.h"
#include <map>

/**
 * UplinkProducer provides operations on an UplinkPacket
 * An UplinkProducer is stateless
 */
class UplinkProducer : public Uplink{
  public:

    UplinkProducer(StateFieldRegistry& r);

    /**
     * Creates an UplinkPacket from given json file
     * Discards any old data in the packet and overwrites with data in filename
     */
    void create_from_json(bitstream& bs, const std::string& filename);

    /**
     * Add an entry to the bitstream
     * @return the number of bits written
     */ 
    size_t add_entry(bitstream& bs, char* val, size_t index);

    /**
     * Prints the UplinkPacket in format:
     * [Index][Data] (field name) (field size)
     */
    void to_string(const bitstream& bs);

    /**
     * Verifies then saves the UplinkPacket in the file
     */
    void to_file(const bitstream& bs, const std::string& filename);

  private:
    MainControlLoop fcp;
    // maps field names to indices
    std::map<std::string, size_t> field_map;
};
