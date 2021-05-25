#ifdef DESKTOP

#include "EEPROMController.hpp"
#include <json.hpp>
#include <iostream>
#include <fstream>
#include <csignal>
#include "mission_state_t.enum"

nlohmann::json EEPROMController::data;

EEPROMController::EEPROMController(StateFieldRegistry &registry)
    : TimedControlTask<void>(registry, "eeprom_ct")
{
    std::ifstream in("eeprom.json");
    if (!in.fail()) in >> data;
    in.close();

    std::signal(SIGTERM, EEPROMController::save_data);
    std::signal(SIGINT, EEPROMController::save_data);
}

void EEPROMController::read_EEPROM(){
  for (unsigned int i = 0; i < _registry.eeprom_saved_fields.size(); i++) {
    const std::string& field_name = _registry.eeprom_saved_fields[i]->name();
    if (data.find(field_name) != data.end()) {
      const unsigned int field_val = data[field_name];

      const bool is_docking = field_val == static_cast<unsigned int>(mission_state_t::docking);
      const bool is_docked = field_val == static_cast<unsigned int>(mission_state_t::docked);
      if (field_name == "pan.state" && !is_docking && !is_docked) continue;

      _registry.eeprom_saved_fields[i]->set_from_eeprom(field_val);
    }
  }
}

void EEPROMController::update_EEPROM(unsigned int position) {
  const std::string& field_name = _registry.eeprom_saved_fields[position]->name();
  const unsigned int field_val = _registry.eeprom_saved_fields[position]->get_eeprom_repr();
  data[field_name] = field_val;
}

bool EEPROMController::check_empty() {
  return data.size() == 0;
}

void EEPROMController::save_data(int signal) {
  std::ofstream out("eeprom.json");
  out << data;
  out.close();

  #ifndef UNIT_TEST
    std::exit(0);
  #endif
}

#endif
