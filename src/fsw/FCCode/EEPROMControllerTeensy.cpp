#ifndef DESKTOP

#include "EEPROMController.hpp"
#include <EEPROM.h>
#include "mission_state_t.enum"

EEPROMController::EEPROMController(StateFieldRegistry &registry)
    : TimedControlTask<void>(registry, "eeprom_ct")
{}

void EEPROMController::read_EEPROM()
{
  for (unsigned int i = 0; i < _registry.eeprom_saved_fields.size(); i++)
  {
    unsigned int field_val = EEPROM.read(addresses[i]);

    const bool is_docking = field_val == static_cast<unsigned int>(mission_state_t::docking);
    const bool is_docked = field_val == static_cast<unsigned int>(mission_state_t::docked);
    if (_registry.eeprom_saved_fields[i]->name() == "pan.state" && !is_docking && !is_docked)
      continue;

    _registry.eeprom_saved_fields[i]->set_from_eeprom(field_val);
  }
}

void EEPROMController::update_EEPROM(unsigned int position)
{
  if (EEPROM.read(addresses[position]) != _registry.eeprom_saved_fields[position]->get_eeprom_repr())
  {
    EEPROM.update(addresses[position], _registry.eeprom_saved_fields[position]->get_eeprom_repr());
  }
}

bool EEPROMController::check_empty()
{
  for (int i = 0; i < EEPROM.length(); i++)
  {
    if (EEPROM.read(i) != 255)
    {
      return false;
    }
  }
  return true;
}

#endif
