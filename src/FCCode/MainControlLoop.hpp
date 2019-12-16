#ifndef MAIN_CONTROL_LOOP_TASK_HPP_
#define MAIN_CONTROL_LOOP_TASK_HPP_

#include "ControlTask.hpp"
#include <StateField.hpp>
#include <StateFieldRegistry.hpp>

#include "ClockManager.hpp"
#include "PiksiControlTask.hpp"
#include "ADCSBoxMonitor.hpp"
#include "AttitudeEstimator.hpp"
#include "GomspaceController.hpp"
#include "DebugTask.hpp"
#include "FieldCreatorTask.hpp"
#include "MissionManager.hpp"
#include "QuakeManager.h"
#include "DockingController.hpp"
#include "DownlinkProducer.hpp"

#if (!defined(FUNCTIONAL_TEST) && !defined(FLIGHT))
static_assert(false, "Need to define either the FUNCTIONAL_TEST or FLIGHT flags.");
#endif

class MainControlLoop : public ControlTask<void> {
   protected:
    FieldCreatorTask field_creator_task;
    ClockManager clock_manager;
    DebugTask debug_task;

    Devices::Piksi piksi;
    PiksiControlTask piksi_control_task;

    Devices::ADCS adcs;
    ADCSBoxMonitor adcs_monitor;
    AttitudeEstimator attitude_estimator;

    Devices::Gomspace::eps_hk_t hk;
    Devices::Gomspace::eps_config_t config; 
    Devices::Gomspace::eps_config2_t config2; 
    Devices::Gomspace gomspace;
    GomspaceController gomspace_controller;

    MissionManager mission_manager;

    Devices::DockingSystem docksys;
    DockingController docking_controller;
    DownlinkProducer downlink_producer;
    QuakeManager quake_manager;

    // Control cycle time offsets, in microseconds
    #ifdef FUNCTIONAL_TEST
        static constexpr unsigned int debug_task_offset          =   5500;
        static constexpr unsigned int piksi_control_task_offset  =  55000;
        static constexpr unsigned int adcs_monitor_offset        =  70500;
        static constexpr unsigned int attitude_estimator_offset  =  85500;
        static constexpr unsigned int gomspace_controller_offset = 106500;
        static constexpr unsigned int mission_manager_offset     = 111600;
        static constexpr unsigned int docking_controller_offset  = 152400;
        static constexpr unsigned int downlink_producer_offset   = 153400;
        static constexpr unsigned int quake_manager_offset       = 153500;
    #else
        static constexpr unsigned int debug_task_offset          =   5500;
        static constexpr unsigned int piksi_control_task_offset  =   6000;
        static constexpr unsigned int adcs_monitor_offset        =  20500;
        static constexpr unsigned int attitude_estimator_offset  =  35500;
        static constexpr unsigned int gomspace_controller_offset =  56500;
        static constexpr unsigned int mission_manager_offset     =  61600;
        static constexpr unsigned int docking_controller_offset  = 103400;
        static constexpr unsigned int downlink_producer_offset   = 104400;
        static constexpr unsigned int quake_manager_offset       = 104500;
    #endif

    /**
     * @brief Total memory use, in bytes.
     */
    ReadableStateField<unsigned int> memory_use_f;

   public:
    /*
     * @brief Construct a new Main Control Loop Task object
     * 
     * @param registry State field registry
     * @param flow_data Metadata for telemetry flows.
     */
    MainControlLoop(StateFieldRegistry& registry,
        const std::vector<DownlinkProducer::FlowData>& flow_data);

    /**
     * @brief Processes state field commands present in the serial buffer.
     */
    void execute() override;

    #ifdef GSW
        /**
         * @brief This function allows ground software to access the downlink.
         */
        DownlinkProducer* get_downlink_producer();
    #endif
};

#endif