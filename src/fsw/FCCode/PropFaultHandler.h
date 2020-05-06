//
// Created by athena on 4/21/20.
//

#ifndef _PROPFAULTHANDLER_H
#define _PROPFAULTHANDLER_H

#include "SimpleFaultHandler.hpp"
#include "Drivers/PropulsionSystem.hpp"

// PropFaultHandler handles faults that were detected in PropController
class PropFaultHandler : public FaultHandlerMachine
{
public:
    PropFaultHandler(StateFieldRegistry &r);
    fault_response_t execute() override;

private:
    WritableStateField<unsigned int> *prop_state_fp;
    // To be used when handling the event where both Tanks want to vent
    WritableStateField<unsigned int> *max_venting_cycles_fp;

    WritableStateField<unsigned int> *max_pressurizing_cycles;

    // ReadableStateField<float>* tank2_pressure_fp;
    // ReadableStateField<float>* tank2_temp_fp;
    // ReadableStateField<float>* tank1_temp_fp;

    Fault *pressurize_fail_fault_fp; // underpressurized
    Fault *overpressure_fault_fp;    // overpressurized
    Fault *tank2_temp_high_fault_fp;
    Fault *tank1_temp_high_fault_fp;

    // If both tanks want to vent, we decrease cooling to 1 second
    // Decrease max_cycles to 1 cycle
    void handle_both_tanks_want_to_vent();
    void handle_restore_values();
    bool both_tanks_want_to_vent() const;

    size_t saved_max_pressurizing_cycles = 0;
    // The number of open-close cycles for which we have been venting both
    // tanks
    size_t num_cycles_both_venting = 0;
#ifdef UNIT_TEST
    friend class TestFixture;
#endif
};

#endif //_PROPFAULTHANDLER_H
