#include "EEPROMController.hpp"

EEPROMController::EEPROMController(StateFieldRegistry &registry, unsigned int offset)
    : TimedControlTask<void>(registry, offset)
{
  mission_mode_fp = find_writable_field<unsigned char>("pan.mode", __FILE__, __LINE__);
  assert(mission_mode_fp);

  is_deployed_fp = find_readable_field<bool>("pan.deployed", __FILE__, __LINE__);
  assert(is_deployed_fp);

  sat_designation_fp = find_writable_field<unsigned char>("pan.sat_designation", __FILE__, __LINE__);
  assert(sat_designation_fp);

  control_cycle_count_fp = find_readable_field<unsigned int>("pan.cycle_no", __FILE__, __LINE__);
  assert(control_cycle_count_fp);
}

void EEPROMController::execute() {
  //if enough control cycles have passed, write the field values to EEPROM
  if(control_cycle_count_fp->get()%period==0){
    writeEEPROM();
  }
  //if the satellite just restarted, read the value from EEPROM and set the 
  //statefield values appropriately by calling readEEPROM()
}

void EEPROMController::readEEPROM(){
  mission_mode_fp->set(EEPROM.read(mission_mode_address));
  is_deployed_fp->set(EEPROM.read(is_deployed_address));
  sat_designation_fp->set(EEPROM.read(sat_designation_address));
  control_cycle_count_fp->set(EEPROM.read(control_cycle_count_address));
}

void EEPROMController::writeEEPROM(){
  EEPROM.write(mission_mode_address, mission_mode_fp->get());
  EEPROM.write(is_deployed_address, is_deployed_fp->get());
  EEPROM.write(sat_designation_address, sat_designation_fp->get());
  EEPROM.write(control_cycle_count_address, control_cycle_count_fp->get());
}