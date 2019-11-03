#include "DockingController.hpp"

DockingController::DockingController(StateFieldRegistry& registry, DockingSystem& docksys)
    : ControlTask<void>(registry), is_turning_sr(0,10,10), is_turning_f("is_turning", is_turning_sr),
    docking_mode_sr(0,10,10), docking_mode_f("docking_mode", docking_mode_sr)
    {
        add_writable_field(is_turning_f);
        is_turning_fp = find_writable_field<unsigned int>("is_turning", __FILE__, __LINE__);

        add_writable_field(docking_mode_f);
        docking_mode_fp = find_writable_field<unsigned int>("docking_mode", __FILE__, __LINE__);
    }

    void DockingController::execute() { 
      is_turning_f.set(is_turning_fp->get());
      docking_mode_f.set(docking_mode_fp->get());
    }