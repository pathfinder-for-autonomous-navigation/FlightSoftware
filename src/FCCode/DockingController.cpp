#include "DockingController.hpp"

DockingController::DockingController(StateFieldRegistry& registry)
    : ControlTask<void>(registry), docked_sr(), docked_f("docked", docked_sr), is_turning_sr(), is_turning_f("is_turning", is_turning_sr)
    {
        add_readable_field(docked_f);
        add_readable_field(is_turning_f);
        docking_motor_dock_fp = find_writable_field<unsigned int>("docking_motor_dock", __FILE__, __LINE__);
    }

    void DockingController::execute() { 
      //if mission manager requests to dock the spacecraft and the system isn't already docked, then start docking
      if (docking_motor_dock_fp->get() && !docksys.check_docked()){
        //if the docking system isn't turning, then start docking
        if (!docksys.check_turning()){
          docksys.startDock();
        }
        //if the docking system is turning, then keep going
        else{
          continue;
        }
      }
      //end the docking when the system is docked
      if (docking_motor_dock_fp->get() && docksys.check_docked()){
        docksys.endDock();
      }

      //if mission manager requests to undockthe spacecraft and the system is docked, then start undocking
      if (!(docking_motor_dock_fp->get()) && docksys.check_docked()) {
        //if the docking system isn't turning, then start undocking
        if (!docksys.check_turning()){
          docksys.startUndock();
        }
        //if the docking system is turning, then keep going
        else{
          continue;
        }
      }
      //end the undocking when the system is docked
      if (!docking_motor_dock_fp->get() && !docksys.check_docked()){
        docksys.endDock();
      }
      
      docked_f.set(docksys.check_docked());
      is_turning_f.set(docksys.check_turning());
    }