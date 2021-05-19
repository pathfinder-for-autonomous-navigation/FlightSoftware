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
  /**
   * @brief Sets the docking_config_cmd_fp pointer to point to the 
   * "docksys.config_cmd" writeable field allowing us to set the motor that
   * affects the magnet configuration (true for docked, false for undocked)
   *  
   * @return void
   */
void DockingController::init() {
  docking_config_cmd_fp = find_writable_field<bool>("docksys.config_cmd", __FILE__, __LINE__);
}

  /**
   * @brief Main control task function: 
   * updates state fields, 
   * initializes step angle and step delay,
   * starts docking process if the docking_cmd field is true and we arent 
   * already in the docking configuration
   * 
   * If we have reached the last step (get_steps() == 0) then we flip the value
   * of dock_config and stop the motor stepping timer, 
   * we also set is_turning to false
   *  
   * @return void
   */
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
