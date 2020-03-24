#include "EEPROMController.hpp"

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
