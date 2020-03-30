#include "UplinkProducer.h"
#include <flow_data.hpp>
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

template<typename UnderlyingType>
size_t UplinkProducer::try_add_field(bitstream bs, std::string key, nlohmann::json j) {
    // Get pointer to that field in the registry
    WritableStateField<UnderlyingType>* ptr = dynamic_cast<WritableStateField<UnderlyingType>*>(registry.find_writable_field(key));

    // If the statefield of the given underlying type doesn't exist in the registry, return 0 bits written. Otherwise, get the value of the key
    if (!ptr) return 0;
    UnderlyingType val = j[key];

    // Check that the value specified in the JSON file is within that serializer bounds of the statefield
    UnderlyingType min = ptr->get_serializer_min();
    UnderlyingType max = ptr->get_serializer_max();
    if (val<min || val>max) 
        throw std::runtime_error("Must set statefield: " + key + " to value within serializer bounds. Min: " +
        std::to_string(min) + " Max: "+std::to_string(max));

    // Make sure the value we want to set does not exceed the max possible
    // value that the field can be set to
    size_t field_index=field_map[key];
    uint64_t max_val = (1ul << (get_field_length(field_index) + 1)) - 1;
    if (static_cast<unsigned int>(val) > max_val) {
        throw std::runtime_error("cannot assign " + std::to_string(val) + " to field " + key + ". max value: " + std::to_string(max_val));
        return false;
    }

    ptr->set(val);
    ptr->serialize();

    // Add the updated value to the bitstream
    return add_entry(bs, ptr->get_bit_array(), field_index);
}

template<typename UnderlyingType>
size_t UplinkProducer::try_add_vector_field(bitstream bs, std::string key, nlohmann::json j) {
    // Get pointer to that field in the registry
    using UnderlyingVectorType = std::array<UnderlyingType, 3>;
    WritableStateField<UnderlyingVectorType>* ptr = dynamic_cast<WritableStateField<UnderlyingVectorType>*>(registry.find_writable_field(key));

    // If the statefield of the given underlying type doesn't exist in the registry, return 0 bits written.
    if (!ptr) return 0;
    UnderlyingVectorType vals = j[key];

    // Check that the magnitude of the values in the JSON file is within the statefield's serializer bounds
    UnderlyingType min = ptr->get_serializer_min()[0];
    UnderlyingType max = ptr->get_serializer_max()[0];
    UnderlyingType vector_mag = sqrt(pow(vals[0], 2) + pow(vals[1], 2) + pow(vals[2], 2));
    if (vector_mag>max || vector_mag<min) throw std::runtime_error("Magnitude of vector must be in range [" + 
        std::to_string(min) + "," + std::to_string(max) + "]");

    ptr->set(vals);
    ptr->serialize();

    // Add the updated value to the bitstream
    size_t field_index=field_map[key];
    return add_entry(bs, ptr->get_bit_array(), field_index);
}

template<typename UnderlyingType>
size_t UplinkProducer::try_add_quat_field(bitstream bs, std::string key, nlohmann::json j) {
    // Get pointer to that field in the registry
    static_assert(std::is_same<UnderlyingType, double>::value || std::is_same<UnderlyingType, float>::value,
        "Can't collect quaternion field info for a vector of non-float or non-double type.");
    using UnderlyingQuatType = std::array<UnderlyingType, 4>;
    WritableStateField<UnderlyingQuatType>* ptr = dynamic_cast<WritableStateField<UnderlyingQuatType>*>(registry.find_writable_field(key));

    // If the quaternion statefield of the given underlying type doesn't exist in the registry, return 0 bits written. Otherwise, get the values of the key
    if (!ptr) return 0;
    std::array<UnderlyingType, 4> vals = j[key];

    // Check that the magnitude of the values in the JSON file is 1 ± some margin of error
    UnderlyingType quat_mag = std::sqrt(std::pow(vals[0], 2) + std::pow(vals[1], 2) + std::pow(vals[2], 2) + std::pow(vals[3], 2));
    UnderlyingType error = 1e-10;
    if (std::abs(quat_mag-1)>error) throw std::runtime_error("Magnitude of quaternion must be 1");

    // Set the statefield pointer to the new values.
    ptr->set(vals);
    ptr->serialize();

    // Add the updated value to the bitstream
    size_t field_index=field_map[key];
    return add_entry(bs, ptr->get_bit_array(), field_index);
}

size_t UplinkProducer::try_add_gps_time(bitstream bs, std::string key, nlohmann::json j) {
    // Get pointer to that field in the registry
    WritableStateField<gps_time_t>* ptr = dynamic_cast<WritableStateField<gps_time_t>*>(registry.find_writable_field(key));

    // If the time statefield doesn't exist in the registry, return 0 bits written. Otherwise, get the values of the key
    if (!ptr) return 0;
    unsigned short wn = j[key][0];
    unsigned int tow = j[key][1];
    unsigned long ns = j[key][2];

    // Set the statefield pointer to the new time.
    ptr->set(gps_time_t(wn,tow,ns));
    ptr->serialize();

    // Add the updated value to the bitstream
    size_t field_index=field_map[key];
    return add_entry(bs, ptr->get_bit_array(), field_index);
}

size_t UplinkProducer::add_field_to_bitstream(bitstream bs, std::string key, nlohmann::json j) {
    size_t bits_written = 0;
    bits_written += try_add_field<unsigned int>(bs, key, j);
    bits_written += try_add_field<signed int>(bs, key, j);
    bits_written += try_add_field<unsigned char>(bs, key, j);
    bits_written += try_add_field<signed char>(bs, key, j);
    bits_written += try_add_field<float>(bs, key, j);
    bits_written += try_add_field<double>(bs, key, j);
    bits_written += try_add_field<bool>(bs, key, j);
    bits_written += try_add_vector_field<float>(bs, key, j);
    bits_written += try_add_vector_field<double>(bs, key, j);
    bits_written += try_add_quat_field<float>(bs, key, j);
    bits_written += try_add_quat_field<double>(bs, key, j);
    bits_written += try_add_gps_time(bs, key, j);
    return bits_written;
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

            size_t bits_written = add_field_to_bitstream(bs, key, j);

            if (bits_written == 0) 
                throw std::runtime_error("Unable to find write " + key + " to bitstream.");
        } 

    // Trim the padding off the byte stream so that validate passes
    bs.max_len = ((bs.byte_offset*8 + bs.bit_offset) + 7) / 8;
    // reset bs pointer to point to the front of the stream
    bs.reset();
 }

size_t UplinkProducer::add_entry(bitstream& bs, const bit_array& val, size_t index)
{
    size_t bits_written = 0;
    size_t field_size = get_field_length(index);

    // Check the field size
    if (field_size == 0)
        throw std::runtime_error("[!Fail] UplinkProducer::add_entry: no field at index: " + std::to_string(index));
    
    // Write the index (offset by 1)
    ++index;
    bits_written += bs.editN(index_size, (uint8_t*)&index);

    // Create a temporary bitstream from the bit array
    char tmp [field_size];
    bitstream bs_temp(tmp, field_size);

    // Write bit array/bs_temp to the bitstream
    bits_written += bs.editN(field_size, bs_temp);
   
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