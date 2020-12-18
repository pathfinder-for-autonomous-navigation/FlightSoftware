#ifndef DOWNLINK_TEST_FIXTURE_HPP_
#define DOWNLINK_TEST_FIXTURE_HPP_

#include <gsw/parsers/src/DownlinkParser.hpp>
#include <gsw/parsers/src/TelemetryInfoGenerator.hpp>
#include "../test/StateFieldRegistryMock.hpp"

class DownlinkTestFixture {
  public:
    /**
     * Constructs the items required for the test: the downlink parser
     * and producer, along with all of the data structures that they use.
     */
    DownlinkTestFixture(const TelemetryInfoGenerator::TelemetryInfo& data);

    using test_input_t = std::map<std::string, std::string>;
    using test_output_t = test_input_t;
    struct test_error_t {
      std::string expected;
      std::string actual;
      double tolerance;
    };
    struct test_result_t {
      bool success;
      std::map<std::string, test_error_t> errors;
    };

    /**
     * Take in telemetry data, apply it to state fields and parse the
     * resultant downlink packets.
     */
    void parse(const test_input_t& input, test_output_t& output);

    /**
     * Generate a valid test input.
     */
    void generate_test_input(test_input_t& input) const;

    static void generate_telemetry_info(TelemetryInfoGenerator::TelemetryInfo& info);

    void save_test_data(
      const test_input_t& input,
      const test_output_t& output,
      const test_result_t& errors,
      const std::string& path) const;

    /**
     * Compare the input data to the DownlinkProducer to the output data
     * from the downlink parser.
     */
    void compare(const test_input_t& input,
      const test_output_t& output, test_result_t& result) const;

  protected:
    /**
     * Apply input data to the state fields in the registry.
     */
    void apply_input(const test_input_t& input);

    /**
     * Create state fields for each telemetry field specified in the input.
     */
    void create_state_fields();

  private:
    const TelemetryInfoGenerator::TelemetryInfo& test_data;

    StateFieldRegistryMock registry;
    std::unique_ptr<DownlinkProducer> downlink_producer;
    std::unique_ptr<DownlinkParser> downlink_parser;

    std::shared_ptr<ReadableStateField<unsigned int>> cycle_count_fp;
    InternalStateField<char*>* snapshot_ptr_fp;
    InternalStateField<size_t>* snapshot_size_bytes_fp;
};

void to_json(nlohmann::json& j, const DownlinkTestFixture::test_error_t& e);
void from_json(const nlohmann::json& j, DownlinkTestFixture::test_error_t& e);

#endif
