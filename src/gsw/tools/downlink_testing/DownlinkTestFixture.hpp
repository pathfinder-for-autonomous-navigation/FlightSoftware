#ifndef DOWNLINK_TEST_FIXTURE_HPP_
#define DOWNLINK_TEST_FIXTURE_HPP_

#include <DownlinkProducer.hpp>
#include <TelemetryInfoGenerator.hpp>
#include <DownlinkParser.hpp>

class DownlinkTestFixture {
  public:
    /**
     * Constructs the items required for the test: the downlink parser
     * and producer, along with all of the data structures that they use.
     */
    DownlinkTestFixture(const TelemetryInfoGenerator::TelemetryInfo& data);

    /**
     * Take in telemetry data, apply it to state fields and parse the
     * resultant downlink packets.
     */
    void parse(const test_input_t& input, const test_output_t& output);

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


    struct test_input_t {
      std::map<std::string, std::string> field_name;
      std::map<std::string, std::string> field_type;
      std::map<std::string, std::string> field_value;
    }
    using test_output_t = test_input_t;
    struct test_error_t {
      std::string expected;
      std::string actual;
      double tolerance;
    }
    struct test_result_t {
      bool success;
      std::map<std::string, test_error_t> errors;
    };

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

#endif
