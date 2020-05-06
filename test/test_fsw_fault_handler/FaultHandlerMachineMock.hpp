#ifndef FAULT_HANDLER_MACHINE_MOCK_HPP_
#define FAULT_HANDLER_MACHINE_MOCK_HPP_

#include <fsw/FCCode/FaultHandlerMachine.hpp>

class FaultHandlerMachineMock : public FaultHandlerMachine {
public:
    FaultHandlerMachineMock(StateFieldRegistry& r) : FaultHandlerMachine(r) {}
    fault_response_t execute() override {
    return response;
    }

    void set(fault_response_t r) {
        response = r;
    }
private:
    fault_response_t response;
};

#endif