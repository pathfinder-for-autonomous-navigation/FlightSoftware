#ifndef DESKTOP

#include "EEPROMController.hpp"
#include <EEPROM.h>

EEPROMController::EEPROMController(StateFieldRegistry &registry, unsigned int offset)
    : TimedControlTask<void>(registry, "eeprom_ct", offset)
{}

void EEPROMController::read_EEPROM(){
  for (unsigned int i = 0; i<pointers.size(); i++){
    pointers.at(i)->set(EEPROM.read(addresses.at(i)));
  }
}

void EEPROMController::update_EEPROM(unsigned int position){
  EEPROM.put(addresses.at(position), pointers.at(position)->get());
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
