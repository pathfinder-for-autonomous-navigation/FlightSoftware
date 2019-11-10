#ifndef FLOW_MANAGER_HPP_
#define FLOW_MANAGER_HPP_

#include <ControlTask.hpp>
#include <static_buffers.hpp>

class DownlinkProducer : public ControlTask<void> {
   public:
    /**
     * @brief Flow data object, used in order to specify the
     * flow rate, the fields going into a flow, and the flow
     * ID.
     * 
     * We can create a static list of these and use it to initialize the
     * actual Flow object, which creates pointers to state fields and 
     * contains a function to automatically generate a flow packet.
     */
    struct FlowData {
        unsigned char flow_id;
        std::vector<std::string> field_list;
        unsigned char flow_rate;
    };

    DownlinkProducer(StateFieldRegistry& registry, std::vector<FlowData>& flow_data);
    void execute() override;

  protected:
    /**
     * @brief Flow object, which controls the construction and state of a telemetry
     * flow:
     * - Ensures a flow packet doesn't span more than 70 bytes.
     * - Ensures all fields in in the flow packet are available in the state
     * registry.
     * - The flow counter tracks whether or not a flow packet should be produced
     *   on the current control cycle.
     */
    struct Flow {

        /**
         * @brief Construct a new Flow object
         * 
         * Constructs the flow from the list of state fields
         * and computes the size of the flow.
         * 
         * @param r          State field registry object.
         * @param flow_data  Data about the flow.
         * @param _num_flows Total number of flows. This is used to
         *                   create the flow ID #
         */
        Flow(const StateFieldRegistry& r,
             const FlowData& flow_data,
             const unsigned char _num_flows);

        //! Flow ID #
        Serializer<unsigned char> flow_id;

        //! List of fields within the flow
        std::vector<std::shared_ptr<ReadableStateFieldBase>> field_list;

        //! Rate of flow
        unsigned int flow_rate;
        
        //! Counts up from the last time a flow packet of this kind of flow
        //! was produced. Once a flow packet of this kind of flow is produced,
        //! the counter is reset.
        //!
        //! For example, let's say the flow rate is 5 and we just produced a packet.
        //! This flow counter will increment from 0 to 1,2,3,4,5 on each control cycle.
        //! After reaching 5, execute will reset it back to zero on the next control cycle
        //! and call produce_flow_packet on this flow.
        unsigned int flow_counter;
        
        //! Number of characters in the entire flow packet, including the packet header,
        //! but excluding COBS encoding.
        //! This is computed upon construction of the flow.
        size_t flow_packet_size = 0;

        /**
         * @brief Produces the serialized float packet, writing it to a provided character
         * array.
         * 
         * @param dest Destination character array.
         * 
         * @return One of three possibilities:
         *         - If creation and COBS encoding of the packet was successful, return
         *           the number of bytes written to the destination.
         *         - If the destination pointer was invalid (null), return -1.
         *         - If the destination buffer wasn't large enough to store the packet,
         *           return -2.
         */
        int produce_flow_packet(char* dest);
    };

    std::vector<Flow>& flows;
};

#endif
