#include "DockingController.hpp"

DockingController::DockingController(StateFieldRegistry& registry)
    : ControlTask<void>(registry), docked_sr(), docked_f("docked", docked_sr), is_turning_sr(), is_turning_f("is_turning", is_turning_sr)
    {
        add_readable_field(docked_f);
        add_readable_field(is_turning_f);
        docking_motor_dock_fp = find_writable_field<unsigned int>("docking_motor_dock", __FILE__, __LINE__);
    }

    void DockingController::execute() { 
      //if mission manager requests to dock the spacecraft and the system isn't already docked, then dock
      if (docking_motor_dock_fp->get() && !docksys.check_docked()){
        docksys.startDock();
      }
      //if mission manager requests undock and the system is docked, then undock
      if (!(docking_motor_dock_fp->get()) && docksys.check_docked()) {
        docksys.undock();
      }
      docked_f.set(docksys.check_docked());
      is_turning_f.set(docksys.check_turning());
    }