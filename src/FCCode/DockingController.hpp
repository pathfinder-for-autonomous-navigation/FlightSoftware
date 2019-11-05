#pragma once

#ifndef DOCKING_CONTROLLER_HPP_
#define DOCKING_CONTROLLER_HPP_

#include "../../test/StateFieldRegistryMock.hpp"
#include <ControlTask.hpp>
#include <unity.h>
#include "../../lib/Drivers/DockingSystem.cpp"

class DockingController : public ControlTask<void> {
   public:
    
    //construct a control task object with state field registry and docking system
    #ifndef DESKTOP
     //im getting an error here that ControlTask and DockingSystem are not nonstatic members of base class DockingController???
     DockingController(StateFieldRegistry &registry) : ControlTask<int>(registry), DockingSystem() {}
    #else
     DockingController(StateFieldRegistry &registry);
     Devices::DockingSystem docksys;
    #endif

    void execute() override;

   protected:

    //docking system
    #ifndef DESKTOP
     Devices::DockingSystem docksys;
    #endif

    //shared pointer set by mission manager - tells control task to dock or undock motor
    std::shared_ptr<WritableStateField<bool>> docking_motor_dock_fp;

    //state field returns whether or not the spacecraft are docked with one another
    Serializer<unsigned int> docked_sr;
    ReadableStateField<unsigned int>docked_f;

};

#endif