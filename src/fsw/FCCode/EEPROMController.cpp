#include "EEPROMController.hpp"
#ifndef DESKTOP
#include <EEPROM.h>
#endif

EEPROMController::EEPROMController(StateFieldRegistry &registry, unsigned int offset)
    : TimedControlTask<void>(registry, "eeprom_ct", offset)
{

}

void EEPROMController::init(const std::vector<std::string>& statefields, const std::vector<unsigned int>& periods){
  for (size_t i = 0; i<statefields.size(); i++){
    // copy the string name of the statefield into a char array
    char field[statefields.at(i).length() + 1];
    strcpy(field, statefields.at(i).c_str());
    // get the pointer to that statefield and add it to the pointer array
    pointers.push_back(find_readable_field<unsigned int>(field, __FILE__, __LINE__));
    // add the address of the pointer to the address array
    addresses.push_back(i*5);
  }

  sf_periods=periods;

  // if we find stored information from previous control cycles when the control task 
  // is initialized, then set all the statefields to those stored values
  if (!check_empty()){
    read_EEPROM();
  }
  else{
    // Otherwise, that means we have just started the satellite for the first time 
    // and the EEPROM should be empty
  }
}

void EEPROMController::execute() {
  //if enough control cycles have passed, write the field values to EEPROM
  for (size_t i = 0; i<pointers.size(); i++) {
    if(control_cycle_count%sf_periods.at(i)==0){
      update_EEPROM(i);
    }
  }
}

void EEPROMController::read_EEPROM(){
  #ifndef DESKTOP
  for (unsigned int i = 0; i<pointers.size(); i++){
    pointers.at(i)->set(EEPROM.read(addresses.at(i)));
  }
  #endif
}

void EEPROMController::update_EEPROM(unsigned int position){
  #ifndef DESKTOP
  EEPROM.put(addresses.at(position), pointers.at(position)->get());
  #endif
}

bool EEPROMController::check_empty(){
  #ifndef DESKTOP
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    if (EEPROM.read(i)!=255){
      return false;
    }
  }
  return true;
  #endif
  return false;
}