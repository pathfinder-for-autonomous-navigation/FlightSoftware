#include "DockingController.hpp"

DockingController::DockingController(StateFieldRegistry &registry, Devices::DockingSystem &_docksys)
    : ControlTask<void>(registry), docksys(_docksys), docked_sr(),
      docked_f("docksys.docked", docked_sr),
      dock_config_sr(),
      dock_config_f("docksys.dock_config", dock_config_sr),
      is_turning_sr(),
      is_turning_f("docksys.is_turning", is_turning_sr)
{
  add_readable_field(docked_f);
  add_readable_field(dock_config_f);
  add_readable_field(is_turning_f);
  docking_config_cmd_fp = find_writable_field<bool>("docksys.config_cmd", __FILE__, __LINE__);
}

void DockingController::execute() {
  //MOVE TO DOCKING CONFIGURATION
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
        #ifdef DESKTOP
        docksys.set_dock(true);
        #endif
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
        #ifdef DESKTOP
        docksys.set_dock(false);
        #endif
      }
    }
  }

  //SET STATEFIELDS
  docked_f.set(docksys.check_docked());
}
