#include "../DownlinkParser.hpp"
#include "../../flow_data.hpp"
#include <iostream>
#include <chrono>
#include <thread>

#ifndef UNIT_TEST
int main() {
    StateFieldRegistry reg;
    DownlinkParser dp(reg, PAN::flow_data);
    std::string filename;
    while(true) {
        std::getline(std::cin, filename);
        std::cout << dp.process_downlink_file(filename) << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
#endif
