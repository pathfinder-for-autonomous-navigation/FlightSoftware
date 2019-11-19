#include "DownlinkProducer.hpp"

namespace PAN {
std::vector<DownlinkProducer::FlowData> flow_data = {
    {
        // Flow ID 0: contains the GPS time and position.
        0,
        true,
        {
            "foo1",
            "foo2",
        }
    }
};
} // namespace PAN
