#include "UplinkProducer.h"
#include "../flow_data.hpp"
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <exception>
#define UP_MAX_FILESIZE 

UplinkProducer::UplinkProducer(StateFieldRegistry& r):
    Uplink(r),
    fcp(registry, PAN::flow_data)
 {
    max_possible_packet_size = 0;
    // Setup field_map
    for (size_t i = 0; i < registry.writable_fields.size(); ++i)
    {
        auto w = registry.writable_fields[i];
        field_map[w->name().c_str()] = i;
        max_possible_packet_size += index_size + w->bitsize();
    }
 }

 void UplinkProducer::create_from_json(bitstream& bs, const std::string& filename)
 {    
    using json = nlohmann::json;
    try {
        json j;
        std::ifstream fs (filename);
        fs >> j;
        fs.close();

        // Start writing at the beginning of bs
        bs.reset();
        memset(bs.stream, 0, bs.max_len);
        for (auto& e : j.items())
        {
            if (!bs.has_next())
                throw std::runtime_error("bitstream is not large enough");

            // Check whether the requested field exists
            std::string key = e.key();
            if (field_map.find(key) == field_map.end())
                throw std::runtime_error("field map key not found: " + key);

            // Get the field's index in writable_fields
            size_t field_index = field_map[key];
            auto val = e.value();
            add_entry(bs, reinterpret_cast<char*>(&val), field_index);
        }
    } 
    catch (const std::exception& e)
    {
        std::cout << "[!Fail] UplinkProducer::create_from_json: " << e.what() << std::endl;
    }

    // Trim the padding off the byte stream so that validate passes
    bs.max_len = ((bs.byte_offset*8 + bs.bit_offset) + 7) / 8;
    // reset bs pointer to point to the front of the stream
    bs.reset();
 }

size_t UplinkProducer::add_entry( bitstream& bs, char* val, size_t index)
{
    size_t bits_written = 0;
    size_t field_size = get_field_length(index);

    // Check the field size
    if (field_size == 0)
        throw std::runtime_error("[!Fail] UplinkProducer::add_entry: no field at index: " + std::to_string(index));
    
    // Write the index (offset by 1)
    ++index;
    bits_written += bs.editN(index_size, (uint8_t*)&index);

    // Write the specified number of bits from val
    bits_written += bs.editN(field_size, reinterpret_cast<uint8_t*>(val));
    return bits_written;
}

void UplinkProducer::to_string(const bitstream& bs)
{
    // print to STDOUT
}

void UplinkProducer::to_file(const bitstream& bs, const std::string& filename)
{
    bool is_valid = _validate_packet(const_cast<bitstream&>(bs));
    // Throw exception if verification fails
    if (!is_valid)
    {
        std::cout << "Uplink Producer: Packet you created is not valid" << std::endl;
        return;
    }

    // Write to file
}

size_t UplinkProducer::get_max_possible_packet_size()
{
    return max_possible_packet_size;
}