#pragma once

#ifndef DCDC_CONTROLLER_HPP_
#define DCDC_CONTROLLER_HPP_

#include <common/StateFieldRegistry.hpp>
#include "TimedControlTask.hpp"
#include "Drivers/DCDC.hpp"

class DCDCController : public TimedControlTask<void> {
   public:
    
    /**
     * @brief Construct a new DCDC Controller object
     * 
     * @param registry 
     * @param dcdc converter 
     */
    DCDCController(StateFieldRegistry& registry,
        Devices::DCDC &dcdc);

    /**
     * @brief Commands the ADCS and sph pins on the DCDC converter
     */
    void execute() override;

   protected:
    Devices::DCDC &dcdc;

    //command statefield tells control task to enable/disable ADCS pin for DCDC
    WritableStateField<bool> ADCSMotorDCDC_cmd_f;

    //command statefield tells control task to enable/disable SPH pin that 
    //controls the docking system for DCDC
    WritableStateField<bool> SpikeDockDCDC_cmd_f;

    //command statefield tells control task to enable/disable ADCS and SPH pins for DCDC
    WritableStateField<bool> disable_cmd_f;

    //command statefield tells control task to reset ADCS and SPH pins for DCDC
    WritableStateField<bool> reset_cmd_f;

    //readble field - only reads state from ADCS pin
    ReadableStateField<bool> ADCSMotorDCDC_f;

    //readable field - only reads state from SPH pin
    ReadableStateField<bool> SpikeDockDCDC_f;

};

#endif
