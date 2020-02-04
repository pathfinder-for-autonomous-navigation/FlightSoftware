#include "eeprom_configs.hpp"

const std::vector<std::string> PAN::statefields = {"pan.mode", "pan.deployed", "pan.sat_designation", "pan.cycle_no"};
const std::vector<unsigned int> PAN::periods = {2, 3, 5, 7};