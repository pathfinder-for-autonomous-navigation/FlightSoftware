#include "DockingController.hpp"

DockingController::DockingController(StateFieldRegistry& registry, DockingSystem& docksys)
    : ControlTask<void>(registry), check_mode_sr(0,10,10), check_mode_f("check_mode", check_mode_sr)
    {
        add_writable_field(check_mode_f);
        set_mode_fp = find_writable_field<unsigned int>("set_mode", __FILE__, __LINE__);
    }

    void DockingController::execute() { 
      check_mode_f.set(set_mode_fp->get());
    }