#include "DownlinkParserMock.hpp"
#include <unity.h>
#include <fstream>
#include "../StateFieldRegistryMock.hpp"

// Flow data for test
struct TestFixture {
    // Flight Software and Ground Software control classes
    StateFieldRegistryMock reg;
    std::unique_ptr<DownlinkParserMock> parser;
    ReadableStateField<unsigned int>* cycle_count_fp;
    static const std::vector<DownlinkProducer::FlowData> flow_data;
    DownlinkProducer* producer;

    // Flow field inputs
    std::shared_ptr<ReadableStateField<unsigned int>> foo1_fp;

    // Parsing outputs
    InternalStateField<char*>* snapshot_fp;
    InternalStateField<size_t>* snapshot_size_bytes_fp;

    TestFixture() : reg() {
        // Generate fields required in the flow data
        foo1_fp = reg.create_readable_field<unsigned int>("foo1");
        foo1_fp->set(400);

        parser = std::make_unique<DownlinkParserMock>(reg, flow_data);
        producer = parser->get_downlink_producer();
        cycle_count_fp = reg.find_readable_field_t<unsigned int>("pan.cycle_no");
        assert(cycle_count_fp);
        cycle_count_fp->set(20);

        snapshot_fp = reg.find_internal_field_t<char*>("downlink.ptr");
        snapshot_size_bytes_fp = reg.find_internal_field_t<size_t>("downlink.snap_size");
        assert(snapshot_fp);
        assert(snapshot_size_bytes_fp);
    }
};

const std::vector<DownlinkProducer::FlowData> TestFixture::flow_data = {
    {
        1,
        true,
        {
            "foo1"
        }
    }
};

void test_task_initialization() {
    TestFixture tf;
}

void test_task_execute() {
    TestFixture tf;
    tf.producer->execute();
    const json downlink = tf.parser->process_downlink(
        tf.snapshot_fp->get(), tf.snapshot_size_bytes_fp->get());

    // Test that data is OK
    const unsigned int cycle_no = std::stoi(downlink["data"]["pan.cycle_no"].get<std::string>());
    const unsigned int foo1 = std::stoi(downlink["data"]["foo1"].get<std::string>());
    TEST_ASSERT_EQUAL(tf.cycle_count_fp->get(), cycle_no);
    TEST_ASSERT_EQUAL(tf.foo1_fp->get(), foo1);

    // Test that metadata is OK
    TEST_ASSERT_EQUAL(tf.cycle_count_fp->get(), downlink["metadata"]["cycle_no"]);
    TEST_ASSERT_FALSE(downlink["metadata"]["error"]);
    TEST_ASSERT_EQUAL(1, downlink["metadata"]["flow_ids"].size());
    TEST_ASSERT_EQUAL(1, downlink["metadata"]["flow_ids"][0]);
}


int main() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_task_execute);
    return UNITY_END();
}
