#ifndef DOWNLINK_PRODUCER_HPP_
#define DOWNLINK_PRODUCER_HPP_

#include <debug_console.hpp>

class Downlink : public debug_console {
   public:
    /**
     * @brief Flow data object, used in order to specify the
     * - The flow ID.
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
     * @brief Construct a new Downlink object.
     * 
     * @param r The state field registry.
     * @param flow_data An initializer list of flow data. The order of the flows
     *                  determines their initial priority.
     */
    Downlink(const StateFieldRegistry& r, const std::vector<FlowData>& flow_data);


  protected:
    /**
     * @brief Produces a downlink packet and dumps it into dest.
     * 
     * @param cycle_count_ba The control cycle count.
     * @param dest The destination character buffer.
     */
    void produce_downlink(const unsigned int cycle_count, char* dest) const;

    /**
     * @brief Compute the size of the downlink snapshot.
     */
    size_t compute_downlink_size() const;

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

    /**
     * @brief Actual flow data.
     */
    unsigned int num_active_flows = 0;
    std::vector<Flow> flows;

    /**
     * @brief Toggle a flow on or off.
     */
    void toggle_flow(unsigned char id);

    /**
     * @brief Swap the priorities of the flows with the given IDs. Also swaps
     * their active status.
     */
    void swap_flow_priorities(unsigned char id1, unsigned char id2);
};

#endif
