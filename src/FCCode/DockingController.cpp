#include "DockingController.hpp"

DockingController::DockingController(StateFieldRegistry& registry)
    : ControlTask<void>(registry), set_mode_sr(0,10,10), set_mode_f("set_mode", set_mode_sr)
    {
        check_mode_fp = find_writable_field<unsigned int>("check_mode", __FILE__, __LINE__);
        add_writable_field(set_mode_f);
    }

    void DockingController::execute() { 
      set_mode_f.set(check_mode_fp->get());
    }