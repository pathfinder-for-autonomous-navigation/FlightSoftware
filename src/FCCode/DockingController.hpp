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
    DockingController(StateFieldRegistry& registry, Devices::DockingSystem &docksys);

    /**
    #ifndef DESKTOP
     //im getting an error here that DockingSystem is not a nonstatic member of base class DockingController???
     DockingController(StateFieldRegistry &registry) : ControlTask<void>(registry), docksys();

    #else
     DockingController(StateFieldRegistry &registry);
    #endif
    **/
    void execute() override;

   protected:

    //docking system
    /**
    #ifndef DESKTOP
     Devices::DockingSystem docksys;
    #endif
    **/

    Devices::DockingSystem docksys;

    //shared pointer set by mission manager - tells control task to dock or undock motor
    std::shared_ptr<WritableStateField<bool>> docking_motor_dock_fp;

    //state field returns whether or not the spacecraft are docked with one another
    Serializer<bool> docked_sr;
    ReadableStateField<bool> docked_f;

    //state field returns whether or not the docking system in turning
    Serializer<bool> is_turning_sr;
    ReadableStateField<bool> is_turning_f;

};

#endif