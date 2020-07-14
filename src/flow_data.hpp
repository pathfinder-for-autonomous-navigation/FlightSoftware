#ifndef flow_data_hpp_
#define flow_data_hpp_

#include <vector>
#include <string>

namespace PAN {
    /**
     * @brief Flow data object, used in order to specify the
     * - The flow ID. Note: flow IDs must be greater than zero; a
     *   flow ID of zero is used to determine if a packet contains
     *   no more data.
     * - If this is initially an active flow.
     * - The fields going into a flow.
     * 
     * We can create a static list of these and use it to initialize the
     * actual Flow object, which creates pointers to state fields and 
     * contains a function to automatically generate a flow packet.
     */
    struct FlowData {
        unsigned char id;
        bool is_active;
        std::vector<std::string> field_list;
    };

    extern const std::vector<FlowData> flow_data;
}

#endif
