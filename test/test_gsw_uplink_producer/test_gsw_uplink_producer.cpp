#include "UplinkProducerMock.h"
#include <unity.h>
#include <fstream>

// Flow data for test
// struct TestFixture {
//     // Flight Software and Ground Software control classes
//     StateFieldRegistryMock reg;
//     std::unique_ptr<UplinkProducerMock> producer;
// };

class TestFixture {
  public:
    StateFieldRegistryMock registry;

    std::unique_ptr<UplinkProducer> uplink_producer;
    InternalStateField<size_t>* radio_mt_packet_len_fp;
    InternalStateField<char*>* radio_mt_packet_fp;

    std::shared_ptr<WritableStateField<unsigned int>> cycle_no_fp;
    std::shared_ptr<WritableStateField<unsigned char>> adcs_mode_fp;
    std::shared_ptr<WritableStateField<f_quat_t>> adcs_cmd_attitude_fp;
    std::shared_ptr<ReadableStateField<float>> adcs_ang_rate_fp;
    std::shared_ptr<WritableStateField<float>> adcs_min_stable_ang_rate_fp;
    std::shared_ptr<WritableStateField<unsigned char>> mission_mode_fp;
    std::shared_ptr<WritableStateField<unsigned char>> sat_designation_fp;

    // Test Helper function will map field names to indices
    std::map<std::string, size_t> field_map;
   
   // Make external fields
    char mt_buffer[350];
    
    // Create a TestFixture instance of QuakeManager with the following parameters
    TestFixture() : registry() {
        // Initialize internal fields
        uplink_producer = std::make_unique<UplinkProducer>(registry);
        radio_mt_packet_len_fp = registry.find_internal_field_t<size_t>("uplink.len");
        radio_mt_packet_fp = registry.find_internal_field_t<char*>("uplink.ptr");

        radio_mt_packet_fp->set(mt_buffer);
        field_map = std::map<std::string, size_t>();
    }
};b

void test_task_initialization() {
    TestFixture tf;
}

void test_task_execute() {
  TestFixture tf;
}


int main() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_task_execute);
    return UNITY_END();
}
