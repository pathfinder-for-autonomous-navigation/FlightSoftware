#ifndef PIKSI_FAULT_HANDLER_HPP_
#define PIKSI_FAULT_HANDLER_HPP_

#include "FaultHandlerMachine.hpp"

// TODO add Piksi fault handler

class PiksiFaultHandler : public FaultHandlerMachine {
  public:
    PiksiFaultHandler(StateFieldRegistry& r) : FaultHandlerMachine(r) {}
    fault_response_t execute() override {
      return {false, false};
    }
};

#endif
