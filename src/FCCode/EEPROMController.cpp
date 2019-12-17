#include "EEPROMController.hpp"
#include <EEPROM.h>

EEPROMController::EEPROMController(StateFieldRegistry &registry, unsigned int offset)
    : TimedControlTask<void>(registry, "eeprom_ct", offset)
{
  mission_mode_fp = find_writable_field<unsigned char>("pan.mode", __FILE__, __LINE__);
  assert(mission_mode_fp);

  is_deployed_fp = find_readable_field<bool>("pan.deployed", __FILE__, __LINE__);
  assert(is_deployed_fp);

  sat_designation_fp = find_writable_field<unsigned char>("pan.sat_designation", __FILE__, __LINE__);
  assert(sat_designation_fp);

  control_cycle_count_fp = find_readable_field<unsigned int>("pan.cycle_no", __FILE__, __LINE__);
  assert(control_cycle_count_fp);

  // if we find stored information from previous control cycles when the control task 
  // is initialized, then set all the statefields to those stored values
  if (EEPROM.read(control_cycle_count_address)>0){
    readEEPROM();
  }
  else{
    // Otherwise, that means we have just started the satellite for the first time 
    // and the EEPROM should be empty
  }
}

void EEPROMController::execute() {
  //if enough control cycles have passed, write the field values to EEPROM
  if(control_cycle_count_fp->get()%period==0){
    updateEEPROM();
  }
}

void EEPROMController::readEEPROM(){
  #ifndef DESKTOP
  mission_mode_fp->set(EEPROM.read(mission_mode_address));
  is_deployed_fp->set(EEPROM.read(is_deployed_address));
  sat_designation_fp->set(EEPROM.read(sat_designation_address));
  control_cycle_count_fp->set(EEPROM.read(control_cycle_count_address));
  #endif
}

void EEPROMController::updateEEPROM(){
  #ifndef DESKTOP
  EEPROM.put(mission_mode_address, mission_mode_fp->get());
  EEPROM.put(is_deployed_address, is_deployed_fp->get());
  EEPROM.put(sat_designation_address, sat_designation_fp->get());
  EEPROM.put(control_cycle_count_address, control_cycle_count_fp->get());
  #endif
}