#pragma once

#ifndef EEPROM_CONTROLLER_HPP_
#define EEPROM_CONTROLLER_HPP_

#include <common/StateFieldRegistry.hpp>
#include "TimedControlTask.hpp"
#ifdef DESKTOP
    #include <json.hpp>
#endif

class EEPROMController : public TimedControlTask<void> {
   #ifdef UNIT_TEST
    friend class TestFixture;
   #endif

   public:
    /**
     * @brief Construct a new EEPROM Controller object
     * 
     * @param registry 
     * @param offset
     * @param statefields
     */
    EEPROMController(StateFieldRegistry& registry, unsigned int offset);

    /**
     * @brief Sets up addresses for the set of EEPROM-saved fields.
     */
    void init();

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

    // Number of addresses available in EEPROM.
    TRACKED_CONSTANT_SC(unsigned int, eeprom_size, 4096);

  protected:
    //the locations in the EEPROM in which the field values will be stored
    std::vector<int> addresses;

    #ifdef DESKTOP
        // Store EEPROM data in JSON so that it can be written to a file.
        static nlohmann::json data;
        // Saves EEPROM data to file if FSW program quits.
        static void save_data(int signal);
        // Get EEPROM data in file and store it in "data".
        static void get_file_data();
    #endif
};

#endif
