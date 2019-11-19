#ifndef MAIN_CONTROL_LOOP_TASK_HPP_
#define MAIN_CONTROL_LOOP_TASK_HPP_

#include <ControlTask.hpp>
#include "ClockManager.hpp"
#include "DebugTask.hpp"
#include "FieldCreatorTask.hpp"
#include "MissionManager.hpp"
#include "DockingController.hpp"
#include "DownlinkProducer.hpp"
#include <StateField.hpp>
#include <StateFieldRegistry.hpp>

#ifdef HOOTL
// OK
#elif FLIGHT
// OK
#else
static_assert(false, "Need to define either the HOOTL or FLIGHT flags.");
#endif

/**
 * @brief The main event loop for the Flight Controller on PAN.
 */
class MainControlLoop : public ControlTask<void> {
   protected:
    FieldCreatorTask field_creator_task;
    ClockManager clock_manager;
    DebugTask debug_task;
    MissionManager mission_manager;

    Devices::DockingSystem docksys;
    DockingController docking_controller;
    DownlinkProducer downlink_producer;

    // Control cycle time offsets, in microseconds. These constants
    // are defined in the file Flight Software Cycle.xlsx on the PAN 
    // OneDrive.
    #ifdef HOOTL
        static constexpr unsigned int debug_task_offset         = 1000;
        static constexpr unsigned int mission_manager_offset    = 51000;
        static constexpr unsigned int docking_controller_offset = 52000;
        static constexpr unsigned int downlink_producer_offset  = 152900;
    #else
        static constexpr unsigned int debug_task_offset         = 1000;
        static constexpr unsigned int mission_manager_offset    = 1010;
        static constexpr unsigned int docking_controller_offset = 2010;
        static constexpr unsigned int downlink_producer_offset  = 102900;
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

    #ifdef DESKTOP
        /**
         * @brief This function allows ground software to access the downlink.
         */
        DownlinkProducer* get_downlink_producer();
    #endif
};

#endif
