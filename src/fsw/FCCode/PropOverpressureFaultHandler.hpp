#ifndef PROP_OVERPRESSURE_FAULT_HANDLER_HPP_
#define PROP_OVERPRESSURE_FAULT_HANDLER_HPP_

#include "SimpleFaultHandler.hpp"

class PropOverpressureFaultHandler : public SimpleFaultHandler {
  public:
    PropOverpressureFaultHandler(StateFieldRegistry& r);
    fault_response_t execute() override;
};

#endif
