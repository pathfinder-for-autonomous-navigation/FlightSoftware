#pragma once

#include <fsw/FCCode/MainControlLoop.hpp>
#include <fsw/FCCode/UplinkCommon.h>
#include <map>
#include <json.hpp>

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

    /**
     * Helper function for add_field_to_bitstream.
     * Check that the field/key of a given type is in the statefield registry.
     * If it is, add the value of the field/key to the bitstream
     */
    template<typename UnderlyingType>
    size_t try_add_field(bitstream bs, std::string key, nlohmann::json j);

    /**
     * Helper function for add_field_to_bitstream.
     * Check that the vector statefield of a given type is in the statefield registry.
     * If it is, add the value of the field/key to the bitstream
     */
    template<typename UnderlyingType>
    size_t try_add_vector_field(bitstream bs, std::string key, nlohmann::json j);

    /**
     * Helper function for add_field_to_bitstream.
     * Check that the quaternion statefield of a given type is in the statefield registry.
     * If it is, add the value of the field/key to the bitstream
     */
    template<typename UnderlyingType>
    size_t try_add_quat_field(bitstream bs, std::string key, nlohmann::json j);

    /**
     * Helper function for add_field_to_bitstream.
     * Check that the time statefield of a given type is in the statefield registry.
     * If it is, add the value of the field/key to the bitstream
     */
    size_t try_add_gps_time(bitstream bs, std::string key, nlohmann::json j);

    /**
     * Check that a field is in the registry. If it is, add the value to the bitstream.
     * @return number of bits written if successful
     */
    size_t add_field_to_bitstream(bitstream bs, std::string key, nlohmann::json j);

#ifndef DEBUG
  private:
#endif

    /**
     * Add an entry to the bitstream
     * @throw runtime_error if invalid index is specified
     * @return the number of bits written
     */ 
    size_t add_entry(bitstream& bs, const bit_array& val, size_t index);

    MainControlLoop fcp;

    // maps field names to indices
    std::map<std::string, size_t> field_map;

    size_t max_possible_packet_size;

};
