#include "DownlinkTestFixture.hpp"
#include <unistd.h>
#include <sstream>
#include <fstream>

auto dump_to_file = [](const nlohmann::json &data, const std::string& basepath, const std::string &filename, unsigned int i) {
    std::stringstream pathstream;
    pathstream << basepath << "/" << filename << i << ".txt";
    std::string path = pathstream.str();

    std::ofstream outfile(path);
    outfile << data.dump(4);
    outfile.close();
};

int loop_until_failure(size_t num_iterations, const std::string& basepath)
{
    for (size_t i = 0; i < num_iterations; i++)
    {
        // Generate test data and compute downlink
        TelemetryInfoGenerator::TelemetryInfo info;
        DownlinkTestFixture::generate_telemetry_info(info);

        DownlinkTestFixture dtf(info);
        DownlinkTestFixture::test_input_t input;
        DownlinkTestFixture::test_output_t output;
        DownlinkTestFixture::test_result_t result;
        dtf.generate_test_input(input);
        dtf.parse(input, output);
        dtf.compare(input, output, result);

        if (!result.success)
        {
            // Print all data to a file
            nlohmann::json telem_info;
            to_json(telem_info, info);
            dump_to_file(telem_info, basepath, "teleminfo", i + 1);

            nlohmann::json test_output;
            test_output["input"] = input;
            test_output["result"] = result;
            dump_to_file(test_output, basepath, "output", i + 1);

            std::cout << "Failed on test " << i + 1 << std::endl;
            return 1;
        }
    }
    std::cout << "Completed." << std::endl;
    return 0;
}

void print_all(size_t num_iterations, const std::string& basepath)
{
    for (size_t i = 0; i < num_iterations; i++)
    {
        // Generate test data and compute downlink
        TelemetryInfoGenerator::TelemetryInfo info;
        DownlinkTestFixture::generate_telemetry_info(info);

        nlohmann::json telem_info;
        to_json(telem_info, info);
        dump_to_file(telem_info, basepath, "teleminfo", i + 1);

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
        dump_to_file(test_output, basepath, "output", i + 1);

        std::cout << "Completed " << i + 1 << " of " << num_iterations << " tests" << std::endl;
    }
}

int main(int argc, char** argv) {
    if (argc != 3)
    {
        std::cout << "Usage: program [outputpath] [maximum number of iterations]" << std::endl;
        std::exit(1);
    }
    std::string basepath = argv[1];
    size_t num_iterations = std::stoi(argv[2]);

    // return loop_until_failure(num_iterations, basepath);
    print_all(num_iterations, basepath);
}
