#ifndef flow_data_hpp_
#define flow_data_hpp_

#include <fsw/FCCode/DownlinkProducer.hpp>

namespace PAN {
    extern const std::vector<DownlinkProducer::FlowData> flow_data;
    extern const std::vector<DownlinkProducer::FlowData> startup_flows;
    extern const std::vector<DownlinkProducer::FlowData> detumble_flows;
    extern const std::vector<DownlinkProducer::FlowData> close_approach_flows;
    extern const std::vector<DownlinkProducer::FlowData> docking_docked_flows;
}

#endif
