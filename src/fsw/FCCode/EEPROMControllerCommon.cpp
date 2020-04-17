#include "EEPROMController.hpp"

void EEPROMController::init() {
  assert(_registry.eeprom_saved_fields.size() <= eeprom_size / 5);

  for (size_t i = 0; i<_registry.eeprom_saved_fields.size(); i++){
    // add the address of the pointer to the address array
    addresses.push_back(i*5);
  }

  // if we find stored information from previous control cycles when the control task 
  // is initialized, then set all the statefields to those stored values
  if (!check_empty()) {
    read_EEPROM();
  }
  else {
    // Otherwise, that means we have just started the satellite for the first time 
    // and the EEPROM should be empty
  }
}

void EEPROMController::execute() {
  //if enough control cycles have passed, write the field values to EEPROM
  for (size_t i = 0; i<_registry.eeprom_saved_fields.size(); i++) {
    if(control_cycle_count % _registry.eeprom_saved_fields[i]->eeprom_save_period() == 0) {
      update_EEPROM(i);
    }
  }
}
