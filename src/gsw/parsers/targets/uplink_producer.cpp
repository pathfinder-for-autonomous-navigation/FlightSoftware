#include <gsw/parsers/src/UplinkProducer.h>
#include <flow_data.hpp>
#include <iostream>
#include <chrono>
#include <thread>

#ifndef UNIT_TEST
int main() {
    StateFieldRegistry reg;
    UplinkProducer producer(reg);
    std::string filename;
    while(true) {
        std::getline(std::cin, filename);
        producer.create_sbd_from_json(filename, "uplink.sbd");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
#endif
