#include "DockingController.hpp"

DockingController::DockingController(StateFieldRegistry &registry, unsigned int offset,
    Devices::DockingSystem &_docksys)
    : TimedControlTask<void>(registry, "docking_ct", offset), docksys(_docksys),
      docked_f("docksys.docked", Serializer<bool>()),
      dock_config_f("docksys.dock_config", Serializer<bool>()),
      is_turning_f("docksys.is_turning", Serializer<bool>()),
      enter_docking_cycle_f("docksys.enter_docking_ccno")
{
  add_readable_field(docked_f);
  add_readable_field(dock_config_f);
  add_readable_field(is_turning_f);
  add_internal_field(enter_docking_cycle_f);

  // Set default values
  docked_f.set(false);
  dock_config_f.set(true);
  is_turning_f.set(false);
  enter_docking_cycle_f.set(control_cycle_count);
}

void DockingController::init() {
  docking_config_cmd_fp = find_writable_field<bool>("docksys.config_cmd", __FILE__, __LINE__);
}

void DockingController::execute() {
  //MOVE TO DOCKING CONFIGURATION
  enter_docking_cycle_f.set(control_cycle_count);
  if (docking_config_cmd_fp->get()){
    if (!dock_config_f.get()) {
      //prevents execute() from repeating start dock and setting turning angle to 180
      if (!is_turning_f.get()){
        docksys.start_dock();
        is_turning_f.set(true);
      }
      if (docksys.get_steps() > 0) {
        docksys.step_motor();
      }
      else {
        dock_config_f.set(true);
        is_turning_f.set(false);
      }
    }
  }

  //MOVE TO UNDOCKING CONFIGURATION
  if (!(docking_config_cmd_fp->get())) {
    if (dock_config_f.get()){
      if (!is_turning_f.get()){
        docksys.start_undock();
        is_turning_f.set(true);
      }
      if (docksys.get_steps() > 0) {
        docksys.step_motor();
      }
      else {
        dock_config_f.set(false);
        is_turning_f.set(false);
      }
    }
  }

  //SET STATEFIELDS
  docked_f.set(docksys.check_docked());
}
