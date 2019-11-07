#include "DockingController.hpp"

DockingController::DockingController(StateFieldRegistry &registry, DockingSystem &docksys)
    : ControlTask<void>(registry), docked_sr(), docked_f("docked", docked_sr), is_turning_sr(), is_turning_f("is_turning", is_turning_sr)
    {
        add_readable_field(docked_f);
        add_readable_field(is_turning_f);
        docking_motor_dock_fp = find_writable_field<unsigned int>("docking_motor_dock", __FILE__, __LINE__);
    }

    void DockingController::execute() { 
      //DOCKING
      //if mission manager requests to dock the spacecraft, then start docking
      if (docking_motor_dock_fp->get()){
        //if the docking system isn't docked and isn't turning, then start docking
        if (!docksys.check_docked() && !docksys.check_turning()){
          docksys.startDock();
        }

        //if the docking system isn't docked and is turning, then keep going

        //if the system is docked, then end the docking
        if (docksys.check_docked()) {
          docksys.endDock();
        }
      }

      //UNDOCKING
      if (!(docking_motor_dock_fp->get())) {
        //if the docking system is docked and isn't turning, then start undocking
        if (docksys.check_docked() && !docksys.check_turning()){
          docksys.startUndock();
        }
        //if the docking system is docked and is turning, then keep going

        //if the system is undocked, then end the undocking
        if (!docksys.check_docked()) {
          docksys.endUndock();
        }
      }
      //end the undocking when the system is undocked
      if (!docking_motor_dock_fp->get() && !docksys.check_docked()){
        docksys.endDock();
      }
      
      //SETTING STATEFIELDS
      docked_f.set(docksys.check_docked());
      is_turning_f.set(docksys.check_turning());
    }