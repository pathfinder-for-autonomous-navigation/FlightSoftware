#include "../UplinkProducer.h"
#include "../../flow_data.hpp"
#include <iostream>

#ifndef UNIT_TEST
int main(int argc, char** argv) {{
    StateFieldRegistry reg;
    UplinkProducer producer(reg);

    if (argc < 2) {
        std::cout << "You must specify an input JSON file." << std::endl;
    }
    else if (argc < 3) {
        std::cout << "You must specify an output filename." << std::endl;
    }
    else if (argc > 3) {
        std::cout << "Too many  arguments." << std::endl;
    }

    char packet[70];
    bitstream bs(packet, 70);
    producer.create_from_json(bs, std::string(argv[1]));
    producer.to_file(bs, std::string(argv[2]));
    return 0;
}}
#endif
