#include "DownlinkParserMock.hpp"
#include <unity.h>

// Flow data for test
class TestFixture {
  protected:
    // Flight Software and Ground Software control classes
    StateFieldRegistryMock reg;
    std::unique_ptr<DownlinkParserMock> parser;
    DownlinkProducer* producer;

    // Flow field inputs
    std::shared_ptr<ReadableStateField<gps_time_t>> foo1_fp;
    std::shared_ptr<ReadableStateField<gps_time_t>> foo2_fp;

    // Parsing outputs
    InternalStateField<char*>* snapshot_fp;
    InternalStateField<size_t>* snapshot_size_bytes_fp;

  public:
    static const std::vector<DownlinkProducer::FlowData> flow_data;

    TestFixture() : reg() {
        // Generate fields required in the flow data
        foo1_fp = reg.create_readable_field<gps_time_t>("foo1");
        foo2_fp = reg.create_readable_field<gps_time_t>("foo2");

        auto parser = std::make_unique<DownlinkParserMock>(reg, flow_data);
        producer = parser->get_downlink_producer();

        snapshot_fp = reg.find_internal_field_t<char*>("downlink_producer.mo_ptr");
        snapshot_size_bytes_fp = reg.find_internal_field_t<size_t>("downlink_producer.snap_size");
    }

    DownlinkProducer* get_producer() {
        return producer;
    }
};

const std::vector<DownlinkProducer::FlowData> TestFixture::flow_data = {
    {
        0,
        true,
        {
            "foo1", // 59 bits
            "foo2", // 59 bits
        } // Flow size 119 bits (118 + 1)
    },
    {
        1,
        true,
        {
            "foo1", // 59 bits
            "foo2", // 59 bits
        } // Flow size: 119 bits (118 + 1)
    }
};

void test_task_initialization() {
    TestFixture tf;
}

void test_task_execute() {
    TestFixture tf;
    tf.get_producer()->execute();
}

int test_downlink_parser() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_task_execute);
    return UNITY_END();
}

int main() {
    return test_downlink_parser();
}
