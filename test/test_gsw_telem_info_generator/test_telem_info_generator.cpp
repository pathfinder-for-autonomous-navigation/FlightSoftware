#include "../../src/GroundCode/TelemetryInfoGenerator.hpp"
#include <unity.h>
#include <fstream>

const std::vector<DownlinkProducer::FlowData> flow_data = {
    {
        1,
        true,
        {
            "pan.cycle_no"
        }
    }
};

void test_telem_info_production() {
    TelemetryInfoGenerator generator(flow_data);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_telem_info_production);
    return UNITY_END();
}
