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

    /**
     * @brief Sets the statefields to the values stored in the EEPROM 
     */
    void readEEPROM();

    /**
     * @brief Writes the value of the statefields to the field's 
     * respective address in EEPROM
     */
    void writeEEPROM();

   protected:

    //number of control cycles that must pass before the control task writes to EEPROM
    unsigned int period = 5;

    //shared pointers set by mission manager and the respective locations of the EEPROM
    //in which the field values will be stored
    WritableStateField<unsigned char>* mission_mode_fp;
    unsigned int mission_mode_address=0;

    ReadableStateField<bool>* is_deployed_fp;
    unsigned int is_deployed_address=1;

    WritableStateField<unsigned char>* sat_designation_fp;
    unsigned int sat_designation_address=2;

    ReadableStateField<unsigned int>* control_cycle_count_fp;
    unsigned int control_cycle_count_address=3;

};

#endif
