#ifndef MAIN_FAULT_HANDLER_HPP_
#define MAIN_FAULT_HANDLER_HPP_

#include "FaultHandlerMachine.hpp"
#include <vector>

class MainFaultHandler : public FaultHandlerMachine {
  #ifdef UNIT_TEST
    friend class TestFixtureMainFHBase;
    friend class TestFixtureMainFHMocked;
    friend class TestFixtureMainFHEndToEnd;
  #endif

  public:
    ~MainFaultHandler() = default;

    explicit MainFaultHandler(StateFieldRegistry &registry);

    /**
     * @brief Acquire state fields that hadn't already been added to the
     *        registry upon the time of thew fault handler's construction.
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
    fault_response_t execute() override;

  protected:
    std::vector<std::unique_ptr<FaultHandlerMachine>> fault_handler_machines;

    // Flag that can be used by HOOTL/HITL to disable/enable fault handling
    WritableStateField<bool> fault_handler_enabled_f;
};

#endif
