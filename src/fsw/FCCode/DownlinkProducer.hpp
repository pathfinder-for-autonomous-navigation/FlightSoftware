#ifndef DOWNLINK_PRODUCER_HPP_
#define DOWNLINK_PRODUCER_HPP_

#include "TimedControlTask.hpp"

class DownlinkProducer : public TimedControlTask<void> {
   public:
    static constexpr unsigned int num_bits_in_packet = 560;

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

    /**
     * @brief Construct a new Downlink Producer.
     * 
     * @param registry State field registry.
     * @param offset Offset, in microseconds, from the beginning of the control
     *               task.
     * @param flow_data An initializer list of flow data. The order of the flows
     *                  determines their initial priority.
     */
    DownlinkProducer(StateFieldRegistry& registry,
                     const unsigned int offset);

    /**
     * @brief Initialize flows for the Downlink Producer. This function
     * should be called in the main control loop after the instantiation of all
     * state fields.
     * 
     * @param flow_data 
     */
    void init_flows(const std::vector<FlowData>& flow_data);

    /**
     * @brief Compute the size of the downlink snapshot.
     * 
     * @param If true, this computes the maximum possible size of the snapshot.
     * If false, this computes the size while only considering active flows.
     */
    size_t compute_downlink_size(const bool compute_max = false) const;
    size_t compute_max_downlink_size() const;

    /**
     * @brief Produce flow packets as needed, and keep track of the next
     * most urgent downlink flow group based on the Quake manager's state.
     */
    void execute() override;

    /**
     * @brief Destructor; clears the memory allocated for the snapshot
     * buffer.
     */
    ~DownlinkProducer();

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
             const size_t num_flows);

        //! Flow ID #
        Serializer<unsigned char> id_sr;

        //! If this is an active flow
        bool is_active;

        //! List of fields within the flow
        std::vector<ReadableStateFieldBase*> field_list;

        //! Number of bits in the entire flow packet, including the flow ID.
        size_t get_packet_size() const;
    };

    #ifdef GSW
    const std::vector<Flow>& get_flows() const;
    #endif

    /**
     * @brief Toggle a flow on or off.
     */
    void toggle_flow(unsigned char id);

    /**
     * @brief Swap the priorities of the flows with the given IDs. Also swaps
     * their active status.
     */
    void swap_flow_priorities(unsigned char id1, unsigned char id2);

  protected:
    /** @brief Pointer to cycle count. */
    ReadableStateField<unsigned int>* cycle_count_fp;

    /**
     * @brief Fields used by the Quake manager to know from where to copy a downlink
     * snapshot, and the length of the snapshot.
     */
    char* snapshot = nullptr;
    InternalStateField<char*> snapshot_ptr_f;
    InternalStateField<size_t> snapshot_size_bytes_f;

    /**
     * @brief Actual flow data.
     */
    unsigned int num_active_flows = 0;
    std::vector<Flow> flows;
};

#endif
