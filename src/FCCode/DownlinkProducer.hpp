#ifndef FLOW_MANAGER_HPP_
#define FLOW_MANAGER_HPP_

#include <ControlTask.hpp>

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
        unsigned char id;
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

    ~DownlinkProducer();

  protected:
    /**
     * @brief Fields used by the Quake manager to know from where to copy a downlink
     * snapshot, and the length of the snapshot.
     */
    char* snapshot = nullptr;
    InternalStateField<char*> snapshot_ptr_f;
    InternalStateField<size_t> snapshot_size_bytes_f;

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
         * @param num_flows  Total number of flows. This is used to
         *                   create the flow ID # serializer
         */
        Flow(const StateFieldRegistry& r,
             const FlowData& flow_data,
             const unsigned char num_flows);

        //! Flow ID #
        Serializer<unsigned char> flow_id_sr;

        //! List of fields within the flow
        std::vector<std::shared_ptr<ReadableStateFieldBase>> field_list;
        
        //! Number of characters in the entire flow packet, including the packet header,
        //! but excluding COBS encoding.
        size_t get_packet_size() const;

        /**
         * @brief Produces the serialized float packet, writing it to a provided character
         * array at a given bit offset from the beginning of the array.
         * 
         * @param dest Destination character array.
         * @param offset Bit offset.
         */
        void produce_flow_packet(unsigned char* dest, size_t offset);
    };

    /**
     * @brief Actual flow data.
     */
    std::map<unsigned char, Flow>& flows;
};

#endif
