#ifndef QUAKE_FAULT_HANDLER_HPP_
#define QUAKE_FAULT_HANDLER_HPP_

#include "FaultHandlerMachine.hpp"

// TODO add quake fault handler

class QuakeFaultHandler : public FaultHandlerMachine {
  public:
    QuakeFaultHandler(StateFieldRegistry& r) : FaultHandlerMachine(r) {}
    fault_response_t execute() override {
      return fault_response_t::none;
    }
};

#endif
