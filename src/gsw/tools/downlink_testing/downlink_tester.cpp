#include "DownlinkTestFixture.hpp"
#include <sstream>
#include <fstream>

int main(int argc, char** argv) {
    if (argc != 3)
    {
        std::cout << "Usage: program [outputpath] [number of iterations]" << std::endl;
        std::exit(1);
    }

    std::string basepath = argv[1];

    auto dump_to_file = [&](const nlohmann::json& data, const std::string& filename, unsigned int i) {
        std::stringstream pathstream;
        pathstream << basepath << "/" << filename << i << ".txt";
        std::string path = pathstream.str();

        std::ofstream outfile(path);
        outfile << data.dump(4);
        outfile.close();
    };

    size_t num_iterations = std::stoi(argv[2]);

    for(int i = 0; i < num_iterations; i++)
    {
        // Generate test data and compute downlink
        TelemetryInfoGenerator::TelemetryInfo info;
        DownlinkTestFixture::generate_telemetry_info(info);

        nlohmann::json telem_info;
        to_json(telem_info, info);
        dump_to_file(telem_info, "teleminfo", i+1);

        DownlinkTestFixture dtf(info);
        DownlinkTestFixture::test_input_t input;
        DownlinkTestFixture::test_output_t output;
        DownlinkTestFixture::test_result_t result;
        dtf.generate_test_input(input);
        dtf.parse(input, output);
        dtf.compare(input, output, result);
        
        nlohmann::json test_output;
        test_output["input"] = input;
        test_output["result"] = result;
        dump_to_file(test_output, "output", i+1);

        std::cout << "Completed test " << i + 1 << " of " << num_iterations << std::endl;
    }
}
