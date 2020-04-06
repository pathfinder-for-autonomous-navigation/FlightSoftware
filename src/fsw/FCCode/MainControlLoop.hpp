#ifndef MAIN_CONTROL_LOOP_TASK_HPP_
#define MAIN_CONTROL_LOOP_TASK_HPP_

#include "ControlTask.hpp"
#include <common/StateField.hpp>
#include <common/StateFieldRegistry.hpp>
#include <common/constant_tracker.hpp>

#include "ClockManager.hpp"
#include "PiksiControlTask.hpp"
#include "ADCSBoxMonitor.hpp"
#include "ADCSBoxController.hpp"
#include "AttitudeEstimator.hpp"
#include "AttitudeComputer.hpp"
#include "ADCSCommander.hpp"
#include "GomspaceController.hpp"
#include "DebugTask.hpp"
#include "FieldCreatorTask.hpp"
#include "FaultHandlerMachine.hpp"
#include "MissionManager.hpp"
#include "QuakeManager.h"
#include "DockingController.hpp"
#include "DCDCController.hpp"
#include "DownlinkProducer.hpp"
#include "EEPROMController.hpp"
#include "UplinkConsumer.h"

#if (!defined(FUNCTIONAL_TEST) && !defined(FLIGHT))
static_assert(false, "Need to define either the FUNCTIONAL_TEST or FLIGHT flags.");
#endif

class MainControlLoop : public ControlTask<void> {
   protected:
    FieldCreatorTask field_creator_task;
    ClockManager clock_manager;

    Devices::Piksi piksi;
    PiksiControlTask piksi_control_task;

    Devices::ADCS adcs;
    ADCSBoxMonitor adcs_monitor;

    DebugTask debug_task;

    AttitudeEstimator attitude_estimator;

    Devices::Gomspace::eps_hk_t hk;
    Devices::Gomspace::eps_config_t config; 
    Devices::Gomspace::eps_config2_t config2; 
    Devices::Gomspace gomspace;
    GomspaceController gomspace_controller;

    Devices::DockingSystem docksys;
    DockingController docking_controller;

    DownlinkProducer downlink_producer;
    QuakeManager quake_manager; // Needs downlink packet from Downlink Producer
    UplinkConsumer uplink_consumer; // Needs uplink packet from Quake Manager

    Devices::DCDC dcdc;
    DCDCController dcdc_controller;

    EEPROMController eeprom_controller;

    // Control cycle time offsets, in microseconds
    // Defined in https://cornellprod-my.sharepoint.com/:x:/r/personal/saa243_cornell_edu/_layouts/15/Doc.aspx?sourcedoc=%7B04C55BBB-7AED-410B-AC43-67352393D6D5%7D&file=Flight%20Software%20Cycle.xlsx&action=default&mobileredirect=true&cid=e2b9bd89-7037-47bf-ad2a-fd8b25808939
    #ifdef FUNCTIONAL_TEST
        TRACKED_CONSTANT_SC(unsigned int, piksi_control_task_offset  , 5500);
        TRACKED_CONSTANT_SC(unsigned int, adcs_monitor_offset        , 7500);
        TRACKED_CONSTANT_SC(unsigned int, debug_task_offset          , 35500);
        TRACKED_CONSTANT_SC(unsigned int, attitude_estimator_offset  , 85500);
        TRACKED_CONSTANT_SC(unsigned int, gomspace_controller_offset , 106500);
        TRACKED_CONSTANT_SC(unsigned int, uplink_consumer_offset     , 111500);
        TRACKED_CONSTANT_SC(unsigned int, mission_manager_offset     , 111600);
        TRACKED_CONSTANT_SC(unsigned int, attitude_computer_offset   , 111700);
        TRACKED_CONSTANT_SC(unsigned int, adcs_commander_offset      , 147400);
        TRACKED_CONSTANT_SC(unsigned int, adcs_box_controller_offset , 147900);
        TRACKED_CONSTANT_SC(unsigned int, docking_controller_offset  , 152400);
        TRACKED_CONSTANT_SC(unsigned int, downlink_producer_offset   , 153400); // excel says 152900
        TRACKED_CONSTANT_SC(unsigned int, quake_manager_offset       , 153500);
        TRACKED_CONSTANT_SC(unsigned int, dcdc_controller_offset     , 153500); // fix this later
        TRACKED_CONSTANT_SC(unsigned int, eeprom_controller_offset   , 153500); // fix this later
    #else
        TRACKED_CONSTANT_SC(unsigned int, piksi_control_task_offset  ,   5500);
        TRACKED_CONSTANT_SC(unsigned int, adcs_monitor_offset        ,   7500);
        TRACKED_CONSTANT_SC(unsigned int, debug_task_offset          ,  35000);
        TRACKED_CONSTANT_SC(unsigned int, attitude_estimator_offset  ,  35500);
        TRACKED_CONSTANT_SC(unsigned int, gomspace_controller_offset ,  56500);
        TRACKED_CONSTANT_SC(unsigned int, uplink_consumer_offset     ,  61500);
        TRACKED_CONSTANT_SC(unsigned int, mission_manager_offset     ,  61600);
        TRACKED_CONSTANT_SC(unsigned int, attitude_computer_offset   ,  61700);
        TRACKED_CONSTANT_SC(unsigned int, adcs_commander_offset      ,  97400);
        TRACKED_CONSTANT_SC(unsigned int, adcs_box_controller_offset ,  97900);
        TRACKED_CONSTANT_SC(unsigned int, docking_controller_offset  , 103400); // excel says 102400
        TRACKED_CONSTANT_SC(unsigned int, downlink_producer_offset   , 104400); // excel says 102900
        TRACKED_CONSTANT_SC(unsigned int, quake_manager_offset       , 104500);
        TRACKED_CONSTANT_SC(unsigned int, dcdc_controller_offset     , 153500); // fix this later
        TRACKED_CONSTANT_SC(unsigned int, eeprom_controller_offset   , 153500); // too high?
    #endif

    /**
     * @brief Total memory use, in bytes.
     */
    ReadableStateField<unsigned int> memory_use_f;

    MissionManager mission_manager;

    AttitudeComputer attitude_computer; // needs adcs.state from MissionManager

    ADCSCommander adcs_commander; // will need inputs from computer

    ADCSBoxController adcs_box_controller; // needs adcs.state from MissionManager

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