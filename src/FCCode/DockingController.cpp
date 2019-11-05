#include "DockingController.hpp"

DockingController::DockingController(StateFieldRegistry& registry)
    : ControlTask<void>(registry), docked_sr(0,10,10), docked_f("docked", docked_sr)
    {
        add_readable_field(docked_f);
        docking_motor_dock_fp = find_writable_field<unsigned int>("docking_motor_dock", __FILE__, __LINE__);
    }

    void DockingController::execute() { 
      //if the mission manager says to dock the spacecraft, then call the dock function on the docking system
      if (docking_motor_dock_fp->get()){
        docksys.dock();
      }
      docked_f.set(docksys.check_docked());
    }