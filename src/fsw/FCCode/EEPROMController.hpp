#pragma once

#ifndef EEPROM_CONTROLLER_HPP_
#define EEPROM_CONTROLLER_HPP_

#include <common/StateFieldRegistry.hpp>
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
    EEPROMController(StateFieldRegistry& registry, unsigned int offset, std::vector<std::string>& statefields, std::vector<unsigned int>& periods);

    /**
     * @brief Gets the pointers to the statefields from the statefield
     * registry. Sets the pointer values to those stored in the EEPROM
     * if necessary.
     */
    void init(std::vector<std::string>& statefields);

    /**
     * @brief Writes to the EEPROM after a certain number of 
     * control cycles given by the period.
     */
    void execute() override;

    /**
     * @brief Sets the statefields to the values stored in the EEPROM 
     */
    void read_EEPROM();

    /**
     * @brief Writes the value of the statefields to the field's 
     * respective address in EEPROM
     * @param position refers to the location of the statefield pointer, 
     * its address in the EEPROM, and its period in their respective vectors
     */
    void update_EEPROM(unsigned int position);

    /**
     * @brief Checks if the EEPROM is empty or not. The default value
     * of the EEPROM is 0xFF, or 255 in decimal. Returns true if empty
     * and false if values are stored in the EEPROM.
     */
    bool check_empty();

    //the locations in the EEPROM in which the field values will be stored
    std::vector<int> addresses;

    // Shared pointers to statefields that will be written to the EEPROM
    std::vector<ReadableStateField<unsigned int>*> pointers;

    // Number of control cycles that must pass before the control task writes the value 
    // of a certain statefield to EEPROM
    std::vector<unsigned int> sf_periods;

};

#endif
