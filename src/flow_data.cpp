#include "flow_data.hpp"

const std::vector<DownlinkProducer::FlowData> PAN::flow_data = {
{1, "true", {"pan.state", "pan.deployed", "pan.sat_designation"}},
{2, "true", {"docksys.docked", "docksys.dock_config", "docksys.is_turning"}}
};
