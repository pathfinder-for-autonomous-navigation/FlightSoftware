#include "DockingController.hpp"

DockingController::DockingController(StateFieldRegistry &registry,
    Devices::DockingSystem &_docksys)
    : TimedControlTask<void>(registry, "docking_ct"), docksys(_docksys),
      docking_step_angle_f("docksys.step_angle", Serializer<float>(0, 180, 16)),
      docking_step_delay_f("docksys.step_delay", Serializer<unsigned int>()),
      docked_f("docksys.docked", Serializer<bool>()),
      dock_config_f("docksys.dock_config", Serializer<bool>()),
      is_turning_f("docksys.is_turning", Serializer<bool>())
{
  add_readable_field(docked_f);
  add_readable_field(dock_config_f);
  add_readable_field(is_turning_f);

  add_writable_field(docking_step_angle_f);
  add_writable_field(docking_step_delay_f);

  // Set default values
  docked_f.set(false);
  dock_config_f.set(true);
  is_turning_f.set(false);

  docking_step_delay_f.set(4000);
  docking_step_angle_f.set(0.032f);

  docksys.setup();
}

void DockingController::init() {
  docking_config_cmd_fp = find_writable_field<bool>("docksys.config_cmd", __FILE__, __LINE__);
}

void DockingController::execute() {
  //update writable fields
  docksys.set_step_angle(docking_step_angle_f.get());
  docksys.set_step_delay(docking_step_delay_f.get());

  if (docking_config_cmd_fp->get() != dock_config_f.get()) {
    //prevents execute() from repeating start dock and setting turning angle to 180
    if (!is_turning_f.get()){
      docksys.start_halfturn();
      is_turning_f.set(true);
    }

    #ifdef DESKTOP
    if (docksys.get_steps() > 0) {
      docksys.step_motor();
    }
    #endif

    if (docksys.get_steps() == 0) {
	    dock_config_f.set(!dock_config_f.get());
      docksys.cancel();
      is_turning_f.set(false);
    }
  }
  else {
    docksys.cancel();
    is_turning_f.set(false);
  }

  //SET STATEFIELDS
  docked_f.set(docksys.check_docked());
}
