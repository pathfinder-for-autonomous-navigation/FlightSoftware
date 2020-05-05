/**
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @brief Utility functions for generating binary data for telemetry parsing
 * tests.
 * @date 2020-03-29
 */

#include <json.hpp>
#include <common/Serializer.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

template<typename T>
static bit_array& produce_bits(const std::string& value)
{
    Serializer<T> sz;
    return produce_bits(value, sz);
}

template<typename T>
static bit_array& produce_bits(const std::string& value, Serializer<T>& sz)
{
    T val;
    if (!sz.deserialize(value.c_str(), &val))
    {
        throw std::exception();
    }
    sz.serialize(val);
    return sz.get_bit_array();
}

static const bit_array& produce_bits(nlohmann::json& item) {
    std::string type = item["type"];

    item["value"] = std::string(item["value"]);

    if (type == "bool")
    {
        Serializer<bool> sz;
        return produce_bits(item["value"], sz);
    }
    else if (type == "unsigned int")
    {
        if (item.find("min") == item.end()) item["min"] = 0;
        if (item.find("max") == item.end()) item["max"] = 4294967295;
        if (item.find("bitsize") == item.end()) item["bitsize"] = 32;
        Serializer<unsigned int> sz(item["min"], item["max"], item["bitsize"]);
        return produce_bits(item["value"], sz);
    }
    else if (type == "signed int")
    {
        Serializer<signed int> sz(item["min"], item["max"], item["bitsize"]);
        return produce_bits(item["value"], sz);
    }
    else if (type == "unsigned char")
    {
        if (item.find("min") == item.end()) item["min"] = 0;
        if (item.find("max") == item.end()) item["max"] = 255;
        if (item.find("bitsize") == item.end()) item["bitsize"] = 8;
        Serializer<unsigned char> sz(item["min"], item["max"], item["bitsize"]);
        return produce_bits(item["value"], sz);
    }
    else if (type == "signed char")
    {
        Serializer<signed char> sz(item["min"], item["max"], item["bitsize"]);
        return produce_bits<signed char>(item["value"], sz);
    }
    else if (type == "f_vector_t")
    {
        Serializer<f_vector_t> sz(item["min"], item["max"], item["bitsize"]);
        f_vector_t val;
        sz.deserialize(std::string(item["value"]).c_str(), &val);
        sz.serialize(val);
        return sz.get_bit_array();
    }
    else if (type == "d_vector_t")
    {
        Serializer<d_vector_t> sz(item["min"], item["max"], item["bitsize"]);
        d_vector_t val;
        sz.deserialize(std::string(item["value"]).c_str(), &val);
        sz.serialize(val);
        return sz.get_bit_array();
    }
    else if (type == "f_quat_t")
    {
        return produce_bits<f_quat_t>(item["value"]);
    }
    else if (type == "d_quat_t")
    {
        return produce_bits<d_quat_t>(item["value"]);
    }
    else if (type == "gps_time_t")
    {
        return produce_bits<gps_time_t>(item["value"]);
    }
    else {
        static bit_array b;
        return b;
    }
}

std::stringstream generate_packet(nlohmann::json& description)
{
    if (!description.is_array()) {
        throw std::exception();
    }

    std::stringstream bitstring;

    for(nlohmann::json& item : description)
    {
        // Write the bits in big-endian order to the bitstream.
        const bit_array& item_bits = produce_bits(item);
        size_t bitsize = item_bits.size();
        for(int i = 0; i < bitsize; i++)
        {
            bool bit = item_bits[bitsize - 1 - i];
            bitstring << (bit ? '1' : '0');
        }
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
    jsonfile >> j;
    jsonfile.close();

    std::ofstream outputfile(binaryfile_output, std::ios::binary | std::ios::out);
    std::stringstream binarystream = generate_packet(j);
    outputfile << binarystream.str();
    outputfile.close();
}

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "Need to specify input JSON file." << std::endl;
        return 1;
    }
    if(argc < 3) {
        std::cout << "Need to specify output filename." << std::endl;
        return 1;
    }

    generate_packet(argv[1], argv[2]);
    return 0;
}
