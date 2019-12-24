#pragma once

#ifndef EEPROM_CONTROLLER_HPP_
#define EEPROM_CONTROLLER_HPP_

#include "../../test/StateFieldRegistryMock.hpp"
#include "TimedControlTask.hpp"

class EEPROMController : public TimedControlTask<void> {
   public:

    /**
     * @brief Construct a new EEPROM Controller object
     * 
     * @param registry 
     * @param offset
     * @param statefields
     */
    EEPROMController(StateFieldRegistry& registry, unsigned int offset, std::vector<std::string>& statefields);

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

    /**
     * @brief Checks if the EEPROM is empty or not. The default value
     * of the EEPROM is 0xFF, or 255 in decimal. Returns true if empty
     * and false if values are stored in the EEPROM.
     */
    bool checkEmpty();

    //number of control cycles that must pass before the control task writes to EEPROM
    unsigned int period = 5;

    //the locations in the EEPROM in which the field values will be stored
    std::vector<int> addresses;

    // Shared pointers to statefields that will be written to the EEPROM
    std::vector<ReadableStateField<unsigned int>*> pointers;

    ReadableStateField<unsigned int>* control_cycle_count_fp;

};

#endif
