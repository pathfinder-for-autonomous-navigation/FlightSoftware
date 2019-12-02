#ifndef MAIN_CONTROL_LOOP_TASK_HPP_
#define MAIN_CONTROL_LOOP_TASK_HPP_

#include <ControlTask.hpp>
#include <StateField.hpp>
#include <StateFieldRegistry.hpp>

#include "ClockManager.hpp"
#include "PiksiControlTask.hpp"
#include "AttitudeEstimator.hpp"
#include "GomspaceController.hpp"
#include "DebugTask.hpp"
#include "FieldCreatorTask.hpp"
#include "MissionManager.hpp"
#include "QuakeManager.h"
#include "DockingController.hpp"

#if (!defined(HOOTL) && !defined(FLIGHT))
static_assert(false, "Need to define either the HOOTL or FLIGHT flags.");
#endif

class MainControlLoop : public ControlTask<void> {
   protected:
    FieldCreatorTask field_creator_task;
    ClockManager clock_manager;
    DebugTask debug_task;

    Devices::Piksi piksi;
    PiksiControlTask piksi_control_task;
    AttitudeEstimator attitude_estimator;

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
        static constexpr unsigned int debug_task_offset          =   5500;
        static constexpr unsigned int piksi_control_task_offset  =  55000;
        static constexpr unsigned int attitude_estimator_offset  =  85500;
        static constexpr unsigned int gomspace_controller_offset = 106500;
        static constexpr unsigned int mission_manager_offset     = 111600;
        static constexpr unsigned int docking_controller_offset  = 152400;
        static constexpr unsigned int quake_manager_offset       = 153400;
    #else
        static constexpr unsigned int debug_task_offset          =   5500;
        static constexpr unsigned int piksi_control_task_offset  =   6000;
        static constexpr unsigned int gomspace_controller_offset =  57500;
        static constexpr unsigned int mission_manager_offset     =  62600;
        static constexpr unsigned int docking_controller_offset  = 103400;
        static constexpr unsigned int quake_manager_offset       = 104400;
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
