#include <StateFieldRegistry.hpp>
#include "../StateFieldRegistryMock.hpp"
#include "../../src/FCCode/DownlinkProducer.hpp"
#include <unity.h>

void test_task_initialization() {
    StateFieldRegistryMock registry;

    registry.create_readable_field<unsigned int>("pan.cycle_no");

    registry.create_readable_field<gps_time_t>("foo1");
    registry.create_readable_field<gps_time_t>("foo2");
    std::vector<DownlinkProducer::FlowData> flow_data = {
        {
            0,
            {
                "foo1",
                "foo2",
            }
        }
    };

    DownlinkProducer downlink_producer(registry, 2, flow_data);
    auto snapshot_ptr_fp = registry.find_internal_field_t<char*>("downlink_producer.mo_ptr");
    auto snapshot_size_bytes_fp = registry.find_internal_field_t<size_t>("downlink_producer.snap_size");
    #ifdef DESKTOP
        assert(snapshot_ptr_fp);
        assert(snapshot_size_bytes_fp);
    #endif

    TEST_ASSERT_EQUAL(15, snapshot_size_bytes_fp->get());
}

int test_downlink_producer_task() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_downlink_producer_task();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_downlink_producer_task();
}

void loop() {}
#endif
