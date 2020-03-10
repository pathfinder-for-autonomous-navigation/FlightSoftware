#ifndef FAULT_HANDLER_MACHINE_HPP_
#define FAULT_HANDLER_MACHINE_HPP_

#include "TimedControlTask.hpp"
#include "mission_state_t.enum"
#include "fault_response_t.enum"
#include <common/Fault.hpp>

/**
 * @brief Response structure for fault handler state machines.
 */

/**
 * @brief Abstract class for fault handler state machines, which must
 * return a recommended mission state from their execute function.
 */
class FaultHandlerMachine : public ControlTask<fault_response_t> {
  public:
    FaultHandlerMachine(StateFieldRegistry& r) : ControlTask<fault_response_t>(r) {}
    virtual fault_response_t execute() = 0;
};

#endif
