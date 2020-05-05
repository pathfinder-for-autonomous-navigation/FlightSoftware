/**
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @brief Utility functions for generating binary data for telemetry parsing
 * tests.
 * @date 2020-03-29
 */

#include <lib/fsw/json/json.hpp>
#include <common/Serializer.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

template<typename T>
static bit_array* produce_bits(const std::string& value, T min, T max, size_t bitsize)
{
    Serializer<T> sz(min, max, bitsize);
    return produce_bits(value, sz);
}

template<typename T>
static bit_array* produce_bits(const std::string& value)
{
    Serializer<T> sz;
    return produce_bits(value, sz);
}

template<typename T>
static bit_array* produce_bits(const std::string& value, Serializer<T>& sz)
{
    T val;
    if (!sz.deserialize(value, &val))
    {
        throw std::exception();
    }
    sz.serialize(val);
    return &sz.get_bit_array();
}

static bit_array* produce_bits(const nlohmann:json& item) {
    std::string type = item["type"];
    const bit_array* item_bits = nullptr;

    if (type == "bool")
    {
        item_bits = produce_bits<bool>(item["value"]);
    }
    else if (type == "unsigned int")
    {
        if (item.find("min") == item.end()) item["min"] = 0;
        if (item.find("max") == item.end()) item["max"] = 4294967295;
        if (item.find("bitsize") == item.end()) item["bitsize"] = 32;
        item_bits = produce_bits<bool>(item["value"], item["min"], item["max"], item["bitsize"]);
    }
    else if (type == "signed int")
    {
        item_bits = produce_bits<bool>(item["value"], item["min"], item["max"], item["bitsize"]);
    }
    else if (type == "unsigned char")
    {
        if (item.find("min") == item.end()) item["min"] = 0;
        if (item.find("max") == item.end()) item["max"] = 255;
        if (item.find("bitsize") == item.end()) item["bitsize"] = 8;
        item_bits = produce_bits<bool>(item["value"], item["min"], item["max"], item["bitsize"]);
    }
    else if (type == "signed char")
    {
        item_bits = produce_bits<bool>(item["value"], item["min"], item["max"], item["bitsize"]);
    }
    else if (type == "f_vector_t")
    {
        Serializer<f_vector_t> sz(item["min"], item["max"], item["bitsize"]);
        f_vector_t val;
        sz.deserialize(item["value"], &val);
        sz.serialize(val);
        item_bits = &sz.get_bit_array();
    }
    else if (type == "d_vector_t")
    {
        Serializer<d_vector_t> sz(item["min"], item["max"], item["bitsize"]);
        d_vector_t val;
        sz.deserialize(item["value"], &val);
        sz.serialize(val);
        item_bits = &sz.get_bit_array();
    }
    else if (type == "f_quat_t")
    {
        item_bits = produce_bits<f_quat_t>(item["value"]);
    }
    else if (type == "d_quat_t")
    {
        item_bits = produce_bits<d_quat_t>(item["value"]);
    }
    else if (type == "gps_time_t")
    {
        item_bits = produce_bits<gps_time_t>(item["value"]);
    }

    return item_bits;
}

std::stringstream generate_packet(const nlohmann::json::array& description)
{
    std::stringstream bitstring;

    try 
    {
        for(const nlohmann::json::object& item : description)
        {
            const bit_array* item_bits = produce_bits(item);
            if (!item_bits) {
                throw std::exception();
            }
            else {
                // Write the bits in big-endian order to the bitstream.
                for(int i = (*item_bits).size(); i > 0; i--)
                {
                    bitstring << ((*item_bits)[i] ? '1' : '0');
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        return "Parse error.";
    }

    std::stringstream ret;

    std::string bitstring_str = bitstring.str();
    std::string padded_bitstring = bitstring_str;
    for(unsigned int i = 0; i < 8 - (bitstring_str.length() % 8); i++)
    {
        padded_bitstring += "0";
    }

    for(int i = 0; i < padded_bitstring.length(); i += 8)
    {
        std::bitset<8> bits;
        for(unsigned int j = 0; j < 8; j++)
        {
            bits[8 - j] = padded_bitstring[i + j] == '1'; 
        }
        ret << static_cast<unsigned char>(bits.to_ulong());
    }

    return ret;
}

void generate_packet(const std::string& json_input_filename, const std::string& binaryfile_output) {
    std::ifstream jsonfile(json_input_filename, std::ios::in);
    nlohmann::json j;
    j << jsonfile;
    jsonfile.close();

    std::ofstream outputfile(binaryfile_output, std::ios::binary | std::ios::out);
    std::stringstream binarystream = generate_packet(j);
    outputfile << binarystream;
    outputfile.close();
}
