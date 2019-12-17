#include "UplinkProducer.h"
#include "../flow_data.hpp"
#include <vector>
#include <fstream>
#include <json.hpp>
#define UP_MAX_FILESIZE 0x1000

UplinkProducer::UplinkProducer(StateFieldRegistry& r) : 
    registry(r),
    fcp(registry, PAN::flow_data)
 {
    // Setup field_map
    for (size_t i = 0; i < registry.writable_fields.size(); ++i)
    {
        auto w = registry.writable_fields[i];
        field_map[w->name().c_str()] = i;
    }
    // Calculate the maximum number of bits needed to represent the indices
    for (index_size = 1; (registry.writable_fields.size() + 1) / (1 << index_size) > 0; ++index_size)
    {}
 }

 void UplinkProducer::create_from_json(UplinkPacket& up, const std::string& filename)
 {    
    using json = nlohmann::json;
    try {
        json j;
        std::ifstream fs (filename, std::ios::in | std::ios::binary);
        fs >> j;
        fs.close();

        up.bs.reset();

        // Add each entry to the uplink packet
        for (auto& e : j.items())
        {
            char* val = reinterpret_cast<char*>(e.value());
            size_t index = field_map[e.key()];
            add_entry(up, val, index);
        }
    } 
    catch(std::ifstream::failure e)
    {
        std::cout << "UplinkProducer create_from_json failed: " << e.what() << std::endl;
    }
    catch (exception& e)
    {
        std::cout << "Create Uplink from JSON failed: " << e.what() << std::endl;
    }
 }

size_t UplinkProducer::add_entry( UplinkPacket& out, char* val, size_t index)
{
    size_t bits_written = 0;
    size_t field_size = get_field_length(index);
    // Write the index
    ++index; // indices are offset by 1
    bits_written += out.bs.editN(index_size, (uint8_t*)&index);
    // Write the specified number of bits from val
    bits_written += out.bs.editN(field_size, reinterpret_cast<uint8_t*>(val));
    return bits_written;
}

size_t UplinkProducer::get_field_length(size_t field_index)
{
    if (field_index >= registry.writable_fields.size())
        return 0;
    return registry.writable_fields.at(field_index)->get_bit_array().size();
}

void UplinkProducer::to_string(const UplinkPacket& up)
{
    // print to STDOUT
}

void UplinkProducer::to_file(const UplinkPacket& up, const std::string& filename)
{
    // Verify
    // Throw exception if verification fails

    // Write to file
}