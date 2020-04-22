//
// Created by athena on 4/21/20.
//

#ifndef _PROPFAULTHANDLER_H
#define _PROPFAULTHANDLER_H

#include "SimpleFaultHandler.hpp"
#include "Drivers/PropulsionSystem.hpp"

// PropFaultHandler handles faults that were detected in PropController
class PropFaultHandler : public FaultHandlerMachine {
public:
    PropFaultHandler(StateFieldRegistry& r);

    fault_response_t execute() override;

private:

    WritableStateField<unsigned int>* prop_state_fp;

    ReadableStateField<float>* tank2_pressure_fp;
    ReadableStateField<float>* tank2_temp_fp;
    ReadableStateField<float>* tank1_temp_fp;

    Fault* pressurize_fail_fault_fp; // underpressurized
    Fault* overpressure_fault_fp;    // overpressurized
    Fault* tank2_temp_high_fault_fp;
    Fault* tank1_temp_high_fault_fp;

#ifdef UNIT_TEST
    friend class TestFixture;
#endif
};


#endif //_PROPFAULTHANDLER_H
