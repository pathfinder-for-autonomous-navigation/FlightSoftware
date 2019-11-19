#include "../GroundCode/DownlinkParser.hpp"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    DownlinkParser dp;
    std::string filename;
    while(true) {
        std::getline(std::cin, filename);
        std::cout << dp.process_downlink_file(filename) << std::endl;
        sleep_for(std::chrono::milliseconds(100));
    }
}
