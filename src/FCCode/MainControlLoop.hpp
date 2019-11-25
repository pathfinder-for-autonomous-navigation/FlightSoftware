#ifndef MAIN_CONTROL_LOOP_TASK_HPP_
#define MAIN_CONTROL_LOOP_TASK_HPP_

#include <ControlTask.hpp>
#include <StateField.hpp>
#include <StateFieldRegistry.hpp>

#include "ClockManager.hpp"
#include "PiksiControlTask.hpp"
#include "GomspaceController.hpp"
#include "DebugTask.hpp"
#include "FieldCreatorTask.hpp"
#include "MissionManager.hpp"
#include "QuakeManager.h"
#include "DockingController.hpp"

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
    DebugTask debug_task;

    Devices::Piksi piksi;
    PiksiControlTask piksi_control_task;

    Devices::Gomspace::eps_hk_t hk;
    Devices::Gomspace::eps_config_t config; 
    Devices::Gomspace::eps_config2_t config2; 
    Devices::Gomspace gomspace;
    GomspaceController gomspace_controller;

    MissionManager mission_manager;

    Devices::DockingSystem docksys;
    DockingController docking_controller;

    QuakeManager quake_manager;

    // Control cycle time offsets, in microseconds
    #ifdef HOOTL
        static constexpr unsigned int debug_task_offset          = 1000;
        static constexpr unsigned int piksi_control_task_offset  = 51000;
        static constexpr unsigned int gomspace_controller_offset = 51000;
        static constexpr unsigned int mission_manager_offset     = 51000;
        static constexpr unsigned int quake_manager_offset       = 51100;
        static constexpr unsigned int docking_controller_offset  = 53100;
    #else
        static constexpr unsigned int debug_task_offset          =  1000;
        static constexpr unsigned int piksi_control_task_offset  =  1010;
        static constexpr unsigned int gomspace_controller_offset = 51000;
        static constexpr unsigned int mission_manager_offset     =  1010;
        static constexpr unsigned int quake_manager_offset       =  1110;
        static constexpr unsigned int docking_controller_offset  = 21110;
    #endif

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
