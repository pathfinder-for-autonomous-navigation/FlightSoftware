#ifndef SIMPLE_FAULT_HANDLER_HPP_
#define SIMPLE_FAULT_HANDLER_HPP_

#include "FaultHandlerMachine.hpp"

/**
 * @brief Class for a simple fault handler that depends only on a single
 * fault flag's state.
 */
class SimpleFaultHandler : public FaultHandlerMachine {
  public:
    /**
     * @brief Construct a new Simple Fault Handler.
     * 
     * @param r State field registry.
     * @param f Reference to the fault that this fault handler checks.
     * @param active_states The set of mission states during which this fault
     *                      handler is "active" (able to produce fault response recommendations.)
     * @param rs Recommended state if fault is signaled.
     */
    SimpleFaultHandler(StateFieldRegistry& r, Fault* f,
        const std::vector<mission_state_t>& _active_states,
        mission_state_t rs = mission_state_t::safehold);

    /**
     * @brief Return the recommended state if the fault is faulted,
     * or return an ignorable recommendation if it's not.
     * 
     * This function must be implemented by subclasses. It may make use of the
     * execute_basic() function that is implemented in order to provide the
     * recommended state determination. This function is left open for implementation so
     * that fault handlers can also execute additional actions if required.
     * 
     * @return fault_response_t 
     */
    virtual fault_response_t execute() = 0;

    static const std::vector<std::vector<mission_state_t>> active_state_lists;

  protected:
    /**
     * @brief Determines the recommended state to the main fault handler
     * on the basis of the current mission state and the fault flag.
     * 
     * @return fault_response_t Recommended state (either nothing, standby, or safehold.)
     */
    fault_response_t determine_recommended_state() const;

    const WritableStateField<unsigned char>* mission_state_fp;
    Fault* fault;

  private:
    const std::vector<mission_state_t>& active_states;
    mission_state_t recommended_state;
};

class SuperSimpleFaultHandler : public SimpleFaultHandler {
  public:
    SuperSimpleFaultHandler(StateFieldRegistry& r, Fault* f,
          const std::vector<mission_state_t>& _active_states,
          mission_state_t rs = mission_state_t::safehold);
      
    fault_response_t execute() override;
};

#endif
