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
static bit_array produce_bits(const std::string& value, Serializer<T>& sz)
{
    T val;
    if (!sz.deserialize(value.c_str(), &val))
    {
        throw std::exception();
    }
    sz.serialize(val);
    return sz.get_bit_array();
}

static nlohmann::json string_to_json(std::string input) {
    return nlohmann::json::parse(input);
}

static bit_array produce_bits(nlohmann::json& item) {
    std::string type = item["type"];

    std::stringstream item_val;
    item_val << item["value"];
    item["value"] = item_val.str();

    if (type == "bool")
    {
        Serializer<bool> sz;
        return produce_bits(item["value"], sz);
    }
    else if (type == "unsigned int")
    {
        if (item.find("min") == item.end()) item["min"] = 0;
        if (item.find("max") == item.end()) item["max"] = 4294967295;
        if (item.find("bitsize") == item.end())
            item["bitsize"] = IntegerSerializer<unsigned char>::log2i(
                item["max"].get<unsigned int>() - item["min"].get<unsigned int>());
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
        if (item.find("bitsize") == item.end())
            item["bitsize"] = IntegerSerializer<unsigned char>::log2i(
                item["max"].get<unsigned int>() - item["min"].get<unsigned int>());
        Serializer<unsigned char> sz(item["min"], item["max"], item["bitsize"]);
        return produce_bits(item["value"], sz);
    }
    else if (type == "signed char")
    {
        Serializer<signed char> sz(item["min"], item["max"], item["bitsize"]);
        return produce_bits(item["value"], sz);
    }
    else if (type == "f_vector_t")
    {
        Serializer<f_vector_t> sz(item["min"], item["max"], item["bitsize"]);
        return produce_bits(item["value"], sz);
    }
    else if (type == "d_vector_t")
    {
        Serializer<d_vector_t> sz(item["min"], item["max"], item["bitsize"]);
        return produce_bits(item["value"], sz);
    }
    else if (type == "f_quat_t")
    {
        Serializer<f_quat_t> sz;
        return produce_bits(item["value"], sz);
    }
    else if (type == "d_quat_t")
    {
        Serializer<d_quat_t> sz;
        return produce_bits(item["value"], sz);
    }
    else if (type == "lin::Vector3f")
    {
        Serializer<lin::Vector3f> sz(item["min"], item["max"], item["bitsize"]);
        return produce_bits(item["value"], sz);
    }
    else if (type == "lin::Vector3d")
    {
        Serializer<lin::Vector3d> sz(item["min"], item["max"], item["bitsize"]);
        return produce_bits(item["value"], sz);
    }
    else if (type == "lin::Vector4f")
    {
        Serializer<lin::Vector4f> sz;
        return produce_bits(item["value"], sz);
    }
    else if (type == "lin::Vector4d")
    {
        Serializer<lin::Vector4d> sz;
        return produce_bits(item["value"], sz);
    }
    else if (type == "gps_time_t")
    {
        Serializer<gps_time_t> sz;
        return produce_bits<gps_time_t>(item["value"], sz);
    }
    else {
        return bit_array();
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
        std::string item_bitstring = "";
        const bit_array item_bits = produce_bits(item);
        for(bool bit : item_bits) {
            item_bitstring += (bit ? '1' : '0');
        }
        std::reverse(item_bitstring.begin(), item_bitstring.end());
        bitstring << item_bitstring;
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
    // if(argc < 2) {
    //     std::cout << "Need to specify input JSON file." << std::endl;
    //     return 1;
    // }
    // if(argc < 3) {
    //     std::cout << "Need to specify output filename." << std::endl;
    //     return 1;
    // }

    //generate_packet(argv[1], argv[2]);

    //given json string for one type, output the serialized bits
    nlohmann::json item = string_to_json(argv[1]);
    bit_array serialized = produce_bits(item);

    // Write the bits in big-endian order to the bitstream.
        std::stringstream bitstring;
        std::string item_bitstring = "";
        for(bool bit : serialized) {
            item_bitstring += (bit ? '1' : '0');
        }
        bitstring << item_bitstring;
        std::cout << bitstring.str() << std::endl;
    
    return 0;
}
