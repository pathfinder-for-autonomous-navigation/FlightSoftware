#include "MainControlLoop.hpp"

const std::vector<DownlinkProducer::FlowData>
MainControlLoop::flow_data = {
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
