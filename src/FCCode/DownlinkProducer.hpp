#ifndef FLOW_MANAGER_HPP_
#define FLOW_MANAGER_HPP_

#include <ControlTask.hpp>
#include <static_buffers.hpp>

class DownlinkProducer : public ControlTask<void> {
   protected:
    struct Flow {
        Flow(const std::vector<std::shared_ptr<ReadableStateFieldBase>>& _field_list,
             const unsigned int _flow_rate,
             const unsigned int _flow_counter);

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
        
        //! Number of characters in the entire flow packet, including COBS encoding and
        //! the packet header.
        //! This is computed upon construction of the flow.
        size_t flow_packet_size;

        circular_stack<char*, >

        //! Produces the flow packet 
        void produce_flow_packet();
    };
    std::vector<Flow>& flows;

   public:
    DownlinkProducer(StateFieldRegistry& registry, std::vector<Flow>& flows);
    void execute() override;
};

#endif
