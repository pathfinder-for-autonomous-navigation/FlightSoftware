#pragma once

#ifndef EEPROM_CONTROLLER_HPP_
#define EEPROM_CONTROLLER_HPP_

#include "../../test/StateFieldRegistryMock.hpp"
#include <TimedControlTask.hpp>
#include <EEPROM.h>

class EEPROMController : public TimedControlTask<void> {
   public:

    /**
     * @brief Construct a new Docking Controller object
     * 
     * @param registry 
     * @param offset
     */
    EEPROMController(StateFieldRegistry& registry, unsigned int offset);

    /**
     * @brief Writes to the EEPROM after a certain number of 
     * control cycles given by the period.
     */
    void execute() override;

   protected:

    //number of control cycles that must pass before the control task writes to EEPROM
    unsigned int period = 5;

    //shared pointers set by mission manager
    WritableStateField<unsigned char>* mission_mode_fp;

    ReadableStateField<bool>* is_deployed_fp;

    WritableStateField<unsigned char>* sat_designation_fp;

    ReadableStateField<unsigned int>* control_cycle_count_fp;

};

#endif