#ifndef DOWNLINK_PRODUCER_HPP_
#define DOWNLINK_PRODUCER_HPP_

#include "TimedControlTask.hpp"
#include <common/constant_tracker.hpp>

class DownlinkProducer : public TimedControlTask<void> {
   public:
    TRACKED_CONSTANT_SC(unsigned int, num_bits_in_packet, 560);

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
      Offset, in microseconds, from the beginning of the control
     *               task.
     * @param flow_data An initializer list of flow data. The order of the flows
     *                  determines their initial priority.
     */
    DownlinkProducer(StateFieldRegistry& registry);

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

    void check_fault_signalled();

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

        /**
        * @brief Move assignment operator.
        */
        Flow& operator=(Flow&& rhs) {
            is_active = std::move(rhs.is_active);
            id_sr = std::move(rhs.id_sr);
            for (size_t i = 0; i<rhs.field_list.size(); i++) {
                field_list[i] = rhs.field_list[i];
            }
            return *this;
        }

        /**
        * @brief Copy constructor.
        */
        Flow(const Flow& other) {
            *this = other;
        }

        /**
        * @brief Copy assignment operator.
        */
        Flow& operator=(const Flow& rhs) {
            unsigned char flow_id;
            rhs.id_sr.deserialize(&flow_id);
            is_active = rhs.is_active;
            id_sr = std::move(rhs.id_sr);
            field_list = rhs.field_list;
            return *this;
        }
    };

    #if defined GSW || defined DESKTOP
    const std::vector<Flow>& get_flows() const;
    #endif

    /**
     * @brief Toggle a flow on or off.
     */
    void toggle_flow(unsigned char id);

    /**
     * @brief Shift the priorities of the flows with the given IDs by moving the flow
     * with id1 to the flow with id2's position. Also swaps their active status.
     */
    void shift_flow_priorities(unsigned char id1, unsigned char id2);

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

    /**
     * @brief Fields used to shift flows. Moves the flow with id1 to the flow with 
     * id2's position. Default is <0,0> (No flow can have an id of 0).
     */
     std::unique_ptr<WritableStateField<unsigned char>> shift_flows_id1_fp;
     std::unique_ptr<WritableStateField<unsigned char>> shift_flows_id2_fp;

    /**
     * @brief Statefield used to toggle flow's active status. Default is 0 (no flow can have an id of 0)
     */
    std::unique_ptr<WritableStateField<unsigned char>> toggle_flow_id_fp;

    /**
     * @brief Defines faults we want to check are signalled
     * If a fault is signalled, the telemetry flow with the fault information will be moved up in the flow data vector
     * Faults listed last in this list will end up with a higher flow priority if signalled
     */ 
    std::array<Fault *, 13> const active_faults{
        FIND_FAULT(adcs_monitor.wheel_pot_fault.base),  
        FIND_FAULT(adcs_monitor.wheel3_fault.base),
        FIND_FAULT(adcs_monitor.wheel2_fault.base),
        FIND_FAULT(adcs_monitor.wheel1_fault.base), // this one isnt in flow_data.cpp
        FIND_FAULT(gomspace.low_batt.base),
        FIND_FAULT(prop.tank1_temp_high.base),
        FIND_FAULT(prop.tank2_temp_high.base),
        FIND_FAULT(attitude_estimator.fault.base),
        FIND_FAULT(adcs_monitor.functional_fault.base),
        FIND_FAULT(prop.overpressured.base),
        FIND_FAULT(prop.pressurize_fail.base),
        FIND_FAULT(piksi_fh.dead.base),
        FIND_FAULT(gomspace.get_hk.base)
    };

    /**
     * @brief The id of the flow next to the one containing all the faults.
     */
    unsigned char fault_id;

    /**
     * @brief Checks if the flow with all the faults has been shifted (due to a fault being recently signalled)
     */
    bool faults_flow_shifted;
};

#endif
