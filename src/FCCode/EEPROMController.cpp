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

  readEEPROM();
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
  EEPROM.update(mission_mode_address, mission_mode_fp->get());
  EEPROM.update(is_deployed_address, is_deployed_fp->get());
  EEPROM.update(sat_designation_address, sat_designation_fp->get());
  EEPROM.update(control_cycle_count_address, control_cycle_count_fp->get());
  #endif
}

unsigned int EEPROMController::get_period(){
  return period;
}

unsigned int EEPROMController::get_mission_mode_address(){
  return mission_mode_address;
}

unsigned int EEPROMController::get_is_deployed_address(){
  return is_deployed_address;
}

unsigned int EEPROMController::get_sat_designation_address(){
  return sat_designation_address;
}

unsigned int EEPROMController::get_control_cycle_count_address(){
  return control_cycle_count_address;
}