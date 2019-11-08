#include "DockingController.hpp"

DockingController::DockingController(StateFieldRegistry &registry, Devices::DockingSystem &_docksys)
    : ControlTask<void>(registry), docked_sr(),
      docked_f("docksys.docked", docked_sr),
      dock_config_sr(),
      dock_config_f("docksys.dock_config", dock_config_sr),
      docksys(_docksys)
{
  add_readable_field(docked_f);
  add_readable_field(dock_config_f);
  docking_config_cmd_fp = find_writable_field<unsigned int>("docksys.config_cmd", __FILE__, __LINE__);
}

void DockingController::execute() {
  //if mission manager requests to dock the spacecraft, then start docking

  //DOCKING
  if (docking_config_cmd_fp->get()){
    if (!dock_config_f.get()) {
      docksys.start_dock();
      if (docksys.get_steps() > 0) {
        docksys.step_motor();
      }
      else {
        dock_config_f.set(true);
      }
    }
  }

  //UNDOCKING
  if (!(docking_config_cmd_fp->get())) {
    if (dock_config_f.get()){
      docksys.start_undock();
      if (docksys.get_steps() > 0) {
        docksys.step_motor();
      }
      else {
        dock_config_f.set(false);
      }
    }
  }

  //SETTING STATEFIELDS
  docked_f.set(docksys.check_docked());
}
