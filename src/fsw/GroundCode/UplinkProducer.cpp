#include "UplinkProducer.h"
#include "../flow_data.hpp"
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <exception>

UplinkProducer::UplinkProducer(StateFieldRegistry& r):
    Uplink(r),
    fcp(registry, PAN::flow_data)
 {
    max_possible_packet_size = 0;
    // initialize index_size
    init_uplink();
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
        json j;
        std::ifstream fs (filename);
        fs >> j;
        fs.close();

        // Start writing at the beginning of the bitstream
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

            // Make sure the value we want to set does not exceed the max possible
            // value that the field can be set to
            uint64_t val = e.value();
            uint64_t max_val = (1ul << (get_field_length(field_index) + 1)) - 1;
            if (val > max_val)
                throw std::runtime_error("cannot assign " + std::to_string(val) + " to field " + key + ". max value: " + std::to_string(max_val));

            add_entry(bs, reinterpret_cast<char*>(&val), field_index);
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

    // Write the specified number of    bits from val
    bits_written += bs.editN(field_size, reinterpret_cast<uint8_t*>(val));
   
    return bits_written;
}

void UplinkProducer::print_packet(bitstream& bs)
{
    size_t packet_size = bs.max_len*8;
    std::vector<bool> bit_ar (packet_size, 0);
    size_t field_index = 0, field_len = 0, bits_consumed = 0;
    std::cout << "idx" << "\twidth" << "\tvalue" << std::endl;
    while (bits_consumed < packet_size)
    {
        // Get index from the bitstream
        bits_consumed += bs.nextN(index_size, reinterpret_cast<uint8_t*>(&field_index));
        if (field_index == 0) // reached end of the packet
            return;
        
        --field_index;
        // Get field length from the index
        field_len = get_field_length(field_index);
        auto field_p = registry.writable_fields[field_index];
        std::cout << field_index << "\t" <<field_len;

        // Dump into bit_array
        bit_array ba(field_len);
        bits_consumed += bs.nextN(field_len, ba);
        std::cout << "\t" << field_p->get_bit_array().to_ulong() << " --> " <<  ba.to_ulong();
        std::cout << "\t\t" << field_p->name() << std::endl;
    }
}

void UplinkProducer::to_file(const bitstream& bs, const std::string& filename)
{
    bool is_valid = _validate_packet(const_cast<bitstream&>(bs));
    // Throw exception if verification fails
    if (!is_valid)
       throw std::runtime_error("Uplink Producer: Packet you created is not valid");
    // Write to file
    std::ofstream new_file (filename, std::ios::out | std::ios::binary);
    new_file.write(reinterpret_cast<const char*>(bs.stream), bs.max_len);
    new_file.close();
}

const size_t UplinkProducer::get_max_possible_packet_size()
{
    return max_possible_packet_size;
}

 bool UplinkProducer::create_sbd_from_json(const std::string& json_file, const std::string& dst_file)
 {
    try
    {
        size_t arr_size = get_max_possible_packet_size();
        char tmp [arr_size];
        bitstream bs(tmp, arr_size);
        create_from_json(bs, json_file);
        to_file(bs, dst_file);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Failed to create sbd from json: " << e.what() << '\n';
        return false;
    }
    return true;
 }