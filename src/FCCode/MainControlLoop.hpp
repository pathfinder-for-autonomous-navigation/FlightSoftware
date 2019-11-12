#ifndef MAIN_CONTROL_LOOP_TASK_HPP_
#define MAIN_CONTROL_LOOP_TASK_HPP_

#include <ControlTask.hpp>
#include "ClockManager.hpp"
#include "DebugTask.hpp"
#include "FieldCreatorTask.hpp"
#include "MissionManager.hpp"
#include "DockingController.hpp"
#include <StateField.hpp>
#include <StateFieldRegistry.hpp>

#ifdef HOOTL
// OK
#elif FLIGHT
// OK
#else
static_assert(false, "Need to define either the HOOTL or FLIGHT flags.");
#endif

class MainControlLoop : public ControlTask<void> {
   protected:
    FieldCreatorTask field_creator_task;
    ClockManager clock_manager;
    #ifdef HOOTL
        DebugTask debug_task;
    #endif
    MissionManager mission_manager;

    Devices::DockingSystem docksys;
    DockingController docking_controller;

    // Control cycle time offsets, in microseconds
    #ifdef HOOTL
        static constexpr unsigned int debug_task_offset = 1000;
        static constexpr unsigned int debug_task_duration = 50000;
    #else
        static constexpr unsigned int debug_task_duration = 0;
    #endif
    static constexpr unsigned int mission_manager_offset    = 1000 + debug_task_duration;
    static constexpr unsigned int docking_controller_offset = 2000 + debug_task_duration;

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
