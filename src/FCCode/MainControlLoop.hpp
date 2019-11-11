#ifndef MAIN_CONTROL_LOOP_TASK_HPP_
#define MAIN_CONTROL_LOOP_TASK_HPP_

#include <ControlTask.hpp>
#include "ClockManager.hpp"
#include "DebugTask.hpp"
#include "MissionManager.hpp"
#include "DockingController.hpp"
#include <StateField.hpp>
#include <StateFieldRegistry.hpp>

class MainControlLoop : public ControlTask<void> {
   protected:
    ClockManager clock_manager;
    DebugTask debug_task;
    MissionManager mission_manager;

    Devices::DockingSystem docksys;
    DockingController docking_controller;

    // Control cycle time offsets
    #ifdef DESKTOP
    static constexpr unsigned int SCALE_FACTOR = 1000000; // Milliseconds to nanoseconds
    #else
    static constexpr unsigned int SCALE_FACTOR = 1000; // Milliseconds to microseconds
    #endif
    static constexpr unsigned int debug_task_offset = 1 * SCALE_FACTOR;
    static constexpr unsigned int mission_manager_offset = 51 * SCALE_FACTOR;
    static constexpr unsigned int docking_controller_offset = 53 * SCALE_FACTOR;

   public:
    /**
     * @brief Construct a new Main Control Loop Task object
     * 
     * @param registry State field registry
     */
    MainControlLoop(StateFieldRegistry& registry);

    /**
     * @brief Processes state field commands present in the serial buffer.
     */
    void execute() override;
};

#endif
