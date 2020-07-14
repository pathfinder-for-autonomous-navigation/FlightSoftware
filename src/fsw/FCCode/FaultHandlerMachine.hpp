#ifndef FAULT_HANDLER_MACHINE_HPP_
#define FAULT_HANDLER_MACHINE_HPP_

#include "TimedControlTask.hpp"
#include "mission_state_t.enum.h"
#include "fault_response_t.enum.h"
#include <common/Fault.hpp>

/**
 * @brief Abstract class for fault handler state machines, which must
 * return a recommended mission state from their execute function.
 */
class FaultHandlerMachine : public ControlTask<fault_response_t> {
  public:
    explicit FaultHandlerMachine(StateFieldRegistry& r) : ControlTask<fault_response_t>(r) {}
    FaultHandlerMachine& operator=(const FaultHandlerMachine& other) = delete;
    FaultHandlerMachine& operator=(FaultHandlerMachine&& other) = delete;
    virtual fault_response_t execute() = 0;
};

#endif
