#include <gsw/parsers/src/UplinkProducer.h>
#include <flow_data.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <json.hpp>
#include <fstream>

#ifndef UNIT_TEST
int main() {
    StateFieldRegistry reg;
    UplinkProducer producer(reg);
    std::string json_filename;
    std::string uplink_packet_filename;

    while(true) {
        std::getline(std::cin, json_filename);
        std::getline(std::cin, uplink_packet_filename);
        
        std::ifstream fs (json_filename);
        if (fs) {
            char packet[70];
            bitstream bs(packet, 70);
            try {
                producer.create_from_json(bs, json_filename);
                producer.to_file(bs, uplink_packet_filename);
                std::cout << "{\"status\":\"success\"}";
            }
            catch (const std::exception& e) {
                std::cout << "{\"error\":\"";
                std::cout << e.what();
                std::cout << "\"}";
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
#endif
