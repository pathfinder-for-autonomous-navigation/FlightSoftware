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

        char packet[70];
        bitstream bs(packet, 70);
        
        std::ifstream fs (json_filename);
        if (fs) {
            producer.create_from_json(bs, "telem.json");
            producer.to_file(bs, uplink_packet_filename);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
#endif
