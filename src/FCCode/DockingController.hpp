#pragma once

#ifndef DOCKING_CONTROLLER_HPP_
#define DOCKING_CONTROLLER_HPP_

#include "../../test/StateFieldRegistryMock.hpp"
#include <ControlTask.hpp>
#include <unity.h>
#include "../../lib/Drivers/DockingSystem.cpp"

class DockingController : public ControlTask<void> {
   public:
    
    #ifndef DESKTOP
     DockingController(StateFieldRegistry &registry) : ControlTask<int>(registry), DockingSystem() {}
    #else
     DockingController(StateFieldRegistry &registry);
     Devices::DockingSystem DockSys;
    #endif

    void execute() override;

   protected:

    #ifndef DESKTOP
     Devices::DockingSystem docksys;
    #endif

    std::shared_ptr<WritableStateField<bool>> docking_motor_dock_fp;

    Serializer<unsigned int> docked_sr;
    ReadableStateField<unsigned int>docked_f;

};

#endif