#include "DownlinkTestFixture.hpp"

std::string path = "";

int main() {
    while(true)
    {
        TelemetryInfoGenerator::TelemetryInfo info;
        DownlinkTestFixture::generate_telemetry_info(info);
        DownlinkTestFixture dtf(info);

        while(true)
        {
            DownlinkTestFixture::test_input_t input;
            DownlinkTestFixture::test_output_t output;
            DownlinkTestFixture::test_result_t result;

            dtf.generate_test_input(input);
            dtf.parse(input, output);
            dtf.compare(input, output, result);
            if (!result.success)
                dtf.save_test_data(input, output, result, path);
        }
    }
}
