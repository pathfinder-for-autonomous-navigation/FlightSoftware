#ifndef DESKTOP

#include "EEPROMController.hpp"
#include <EEPROM.h>

EEPROMController::EEPROMController(StateFieldRegistry &registry, unsigned int offset)
    : TimedControlTask<void>(registry, "eeprom_ct", offset)
{}

void EEPROMController::read_EEPROM(){
  for (unsigned int i = 0; i<_registry.eeprom_saved_fields.size(); i++){
    _registry.eeprom_saved_fields[i]->set_from_eeprom(EEPROM.read(addresses[i]));
  }
}

void EEPROMController::update_EEPROM(unsigned int position){
  EEPROM.put(addresses[position], _registry.eeprom_saved_fields[position]->get_eeprom_repr());
}

bool EEPROMController::check_empty(){
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    if (EEPROM.read(i)!=255){
      return false;
    }
  }
  return true;
}

#endif
