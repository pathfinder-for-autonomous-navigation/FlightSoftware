#ifndef FLOW_MANAGER_HPP_
#define FLOW_MANAGER_HPP_

#include <ControlTask.hpp>
#include <static_buffers.hpp>

class DownlinkProducer : public ControlTask<void> {
   public:
    /**
     * @brief Flow data object, used in order to specify the
     * - Flow rate.
     * - The fields going into a flow.
     * - The flow ID.
     * - The maximum number of packets of a particular kind of flow to
     *   save on a stack.
     * 
     * We can create a static list of these and use it to initialize the
     * actual Flow object, which creates pointers to state fields and 
     * contains a function to automatically generate a flow packet.
     */
    struct FlowData {
        unsigned char flow_id;
        unsigned char group_id;
        unsigned char flow_rate;
        unsigned int packet_cache_size;
        std::vector<std::string> field_list;
    };

    /**
     * @brief Construct a new Downlink Producer.
     * 
     * @param registry State field registry.
     * @param flow_data An initializer list of flow data.
     */
    DownlinkProducer(StateFieldRegistry& registry, std::vector<FlowData>& flow_data);

    /**
     * @brief Produce flow packets as needed, and keep track of the next
     * most urgent downlink flow group based on the Quake manager's state.
     */
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
         * @brief Construct a new telemetry flow
         * 
         * Constructs the flow from the list of state fields
         * and checks bounds on the size of the flow packet.
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
        Serializer<unsigned char> flow_id_sr;
        unsigned char group_id;

        //! List of fields within the flow
        std::vector<std::shared_ptr<ReadableStateFieldBase>> field_list;

        //! Rate of flow
        unsigned int rate;
        
        //! Counts up from the last time a flow packet of this kind of flow
        //! was produced. Once a flow packet of this kind of flow is produced,
        //! the counter is reset.
        //!
        //! For example, let's say the flow rate is 5 and we just produced a packet.
        //! This flow counter will increment from 0 to 1,2,3,4,5 on each control cycle.
        //! After reaching 5, execute will reset it back to zero on the next control cycle
        //! and call produce_flow_packet on this flow.
        unsigned int counter;
        
        //! Number of characters in the entire flow packet, including the packet header,
        //! but excluding COBS encoding.
        size_t get_packet_size() const;

        circular_stack<char*, flow_cache_size> flow_packets;

        /**
         * @brief Produces the serialized float packet, writing it to a provided character
         * array.
         * 
         * @param dest Destination character array.
         */
        void produce_flow_packet(unsigned char* dest);
    };

    struct FlowGroup {
        unsigned int group_id;
        std::vector<Flow> flows;
        circular_stack<char*, group_cache_size> downlink_packets;

        /**
         * @brief Compute the downlink size in bytes.
         */
        size_t get_downlink_size() const;

        /**
         * @brief Produces a downlink packet
         * 
         * @param dest Destination character array, writing it to a provided character
         * array
         */
        void produce_downlink_packet(char* dest);
    };

    /**
     * @brief Actual flow data.
     */
    std::map<unsigned char, FlowGroup>& flow_groups;
};

#endif
