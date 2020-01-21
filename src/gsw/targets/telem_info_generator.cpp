#include "../TelemetryInfoGenerator.hpp"
#include "../../flow_data.hpp"
#include <iostream>
#include <fstream>

using namespace std;

#ifndef UNIT_TEST
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Need to specify a filename." << endl;
        return 1;
    }
    if (argc > 2) {
        cout << "Too many arguments." << endl;
        return 1;
    }

    const std::string filename = std::string(argv[1]);
    
    TelemetryInfoGenerator gen(PAN::flow_data);
    const nlohmann::json info = gen.generate_telemetry_info();
    
    std::ofstream output_file(filename);
    output_file << info;
    output_file.close();
    return 0;
}
#endif
