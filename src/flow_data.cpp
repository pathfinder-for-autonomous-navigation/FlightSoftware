#include "flow_data.hpp"

/**
 * When changing this file, remember to do the following steps in sequence as well:
 * - Change the last generator in /test/test_fsw_downlink_producer/packet_gen.py
 * - Run the file in the previous step and get an output.
 * - Save the output in the /test/dat/DownlinkParser/downlink* files
 * - Update /test/dat/DownlinkParser/expected_output.json
 **/

const std::vector<DownlinkProducer::FlowData> PAN::flow_data = {
    {
        1,
        true,
        {
            "pan.state",
            "pan.deployed",
            "pan.sat_designation"
        }
    },
    {
        2,
        true,
        {
            "docksys.docked",
            "docksys.dock_config",
            "docksys.is_turning"
        }
    },
};
