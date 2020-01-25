#pragma once

#ifndef DOCKING_CONTROLLER_HPP_
#define DOCKING_CONTROLLER_HPP_

#include <common/StateFieldRegistry.hpp>
#include "TimedControlTask.hpp"
#include "Drivers/DockingSystem.hpp"

class DockingController : public TimedControlTask<void> {
   public:
    
    /**
     * @brief Construct a new Docking Controller object
     * 
     * @param registry 
     * @param offset
     * @param docksys 
     */
    DockingController(StateFieldRegistry& registry, unsigned int offset,
        Devices::DockingSystem &docksys);

    /**
     * @brief The mission manager creates the docking command field after
     * every control task has been constructed, so this function provides
     * a way for the docking controller to bind to the docking command during
     * main control loop construction.
     */
    void init();

    /**
     * @brief Sets statefield variables and moves the motor to docking/undocking
     * configuration should mission manager request it.
     */
    void execute() override;

   protected:
    Devices::DockingSystem& docksys;

    //shared pointer set by mission manager - tells control task to dock or undock motor
    const WritableStateField<bool>* docking_config_cmd_fp;

    //state field returns whether or not the spacecraft are docked with one another
    ReadableStateField<bool> docked_f;

    //state field returns whether or not the motor is in the docking configuration
    ReadableStateField<bool> dock_config_f;

    //state field returns whether or not the motor is turning/in the process of (un)docking
    ReadableStateField<bool> is_turning_f;

};

#endif
