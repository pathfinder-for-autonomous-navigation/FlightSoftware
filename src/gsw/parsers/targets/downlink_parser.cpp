#include <gsw/parsers/src/DownlinkParser.hpp>
#include <flow_data.hpp>
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

        std::string parse_result = dp.process_downlink_file_json(filename).dump();
        if (parse_result == "null") std::cout << "Error: file not found." << std::endl;
        else std::cout << parse_result << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
#endif
