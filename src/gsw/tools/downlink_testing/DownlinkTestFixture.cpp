#include "DownlinkTestFixture.hpp"

DownlinkTestFixture::DownlinkTestFixture(const TelemetryInfoGenerator::TelemetryInfo& data) : test_data(data)
{
    // Create required field(s)
    cycle_count_fp = registry.create_readable_field<unsigned int>("pan.cycle_no");
    create_state_fields();

    downlink_producer = std::make_unique<DownlinkProducer>(registry, 0);
    downlink_producer->init_flows(data.flow_data);
    snapshot_ptr_fp = registry.find_internal_field_t<char*>("downlink.ptr");
    snapshot_size_bytes_fp = registry.find_internal_field_t<size_t>(
                                "downlink.snap_size");

    downlink_parser = std::make_unique<DownlinkParser>(registry, flow_data);
}

void DownlinkTestFixture::parse(const DownlinkTestFixture::test_input_t& input,
    DownlinkTestFixture::test_output_t& output)
{
    apply_input(input);
    downlink_producer->execute();
    output = downlink_parser->process_downlink_packet(characters);
}

void DownlinkTestFixture::save_test_data(
      const test_input_t& input,
      const test_output_t& output,
      const test_result_t& errors,
      const std::string& path)
{
    
}

void DownlinkTestFixture::generate_test_input(DownlinkTestFixture::test_input_t& input)
{

}

void DownlinkTestFixture::generate_telemetry_info(TelemetryInfoGenerator::TelemetryInfo& info)
{

}

void DownlinkTestFixture::apply_input(const DownlinkTestFixture::test_input_t& input)
{
    // For each field in the input, find it in the registry,
    // give it the value in the input.
}

void DownlinkTestFixture::create_state_fields()
{
}

void DownlinkTestFixture::compare(const DownlinkTestFixture::test_input_t& input,
    const DownlinkTestFixture::test_output_t& output,
    DownlinkTestFixture::test_result_t& result)
{

}
