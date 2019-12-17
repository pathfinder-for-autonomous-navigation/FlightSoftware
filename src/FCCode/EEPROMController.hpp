#pragma once

#ifndef EEPROM_CONTROLLER_HPP_
#define EEPROM_CONTROLLER_HPP_

#include "../../test/StateFieldRegistryMock.hpp"
#include "TimedControlTask.hpp"
#include <EEPROM.h>

class EEPROMController : public TimedControlTask<void> {
   public:

    /**
     * @brief Construct a new EEPROM Controller object
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
    void updateEEPROM();

    //number of control cycles that must pass before the control task writes to EEPROM
    unsigned int period = 5;

    //the locations of the EEPROM in which the field values will be stored
    unsigned int mission_mode_address=0;
    unsigned int is_deployed_address=5;
    unsigned int sat_designation_address=10;
    unsigned int control_cycle_count_address=15;

   protected:
    //shared pointers set by mission manager
    WritableStateField<unsigned char>* mission_mode_fp;

    ReadableStateField<bool>* is_deployed_fp;
    
    WritableStateField<unsigned char>* sat_designation_fp;

    ReadableStateField<unsigned int>* control_cycle_count_fp;

};

#endif
