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
     * @param offset
     * @param dcdc converter 
     */
    DCDCController(StateFieldRegistry& registry, unsigned int offset,
        Devices::DCDC &dcdc);

    /**
     * @brief Sets statefield variables and moves the motor to docking/undocking
     * configuration should mission manager request it.
     */
    void execute() override;

   protected:
    Devices::DCDC &dcdc;

    //command statefield tells control task to enable/disable ADCS pin for DCDC
    WritableStateField<bool> adcs_dcdc_cmd_f;

    //command statefield tells control task to enable/disable SPH pin that 
    //controls the docking system for DCDC
    WritableStateField<bool> sph_dcdc_cmd_f;

    //command statefield tells control task to enable/disable ADCS and SPH pins for DCDC
    WritableStateField<bool> disable_cmd_f;

    //command statefield tells control task to reset ADCS and SPH pins for DCDC
    WritableStateField<bool> reset_cmd_f;

};

#endif
