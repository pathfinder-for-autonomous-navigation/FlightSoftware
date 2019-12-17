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
  
}
