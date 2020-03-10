#ifndef MAIN_FAULT_HANDLER_HPP_
#define MAIN_FAULT_HANDLER_HPP_

#include "FaultHandlerMachine.hpp"
#include <vector>

class MainFaultHandler : public FaultHandlerMachine {
  #ifdef UNIT_TEST
    friend class TestFixtureMainFH;
  #endif

  public:
    /**
     * @brief Construct a new Fault Handler
     * 
     * @param r State field registry.
     */
    MainFaultHandler(StateFieldRegistry& r);

    /**
     * @brief Acquire state fields that hadn't already been added to the registry
     * upon the time of thew fault handler's construction.
     */
    void init();

    /**
     * @brief Steps through all of the underlying fault state machines and
     * combines their recommended mission state outputs.
     * 
     * If the recommended mission state from any underlying fault machine is safehold,
     * we immediately return that state. Otherwise, recommend standby if any fault
     * machine recommends standby. Otherwise, return no recommendation.
     */
    fault_response_t execute();

  protected:
    std::vector<std::unique_ptr<FaultHandlerMachine>> fault_handler_machines;

    // Flag that can be used by HOOTL/HITL to disable/enable fault handling
    WritableStateField<bool> fault_handler_enabled_f;
};

#endif
