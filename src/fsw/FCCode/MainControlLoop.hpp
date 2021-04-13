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
#include "AttitudeController.hpp"
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
#include "PropController.hpp"
#include "OrbitController.hpp"

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

    /**
     * @brief Total memory use, in bytes.
     */
    ReadableStateField<unsigned int> memory_use_f;

    /**
     * @brief Contains the number of control cycles in a 24-hour period.
     * 
     * This field is only used informationally by PTest to correctly time
     * its testcases.
     */
    ReadableStateField<unsigned int> one_day_ccno_f;

    /**
     * @brief Contains the length of a control cycle in milliseconds.
     * 
     * This field is only used informationally by PTest to correctly time
     * its testcases, and potentially by pre-flight check software to
     * verify we've loaded the correct version of flight software to the
     * spacecraft.
     */
    ReadableStateField<unsigned int> control_cycle_ms_f;

    PropController prop_controller;
    MissionManager mission_manager;
    AttitudeController attitude_controller; // needs adcs.state from MissionManager
    ADCSCommander adcs_commander; // will need inputs from computer
    ADCSBoxController adcs_box_controller; // needs adcs.state from MissionManager

    OrbitController orbit_controller;

    // Control cycle time offsets, in microseconds
    #ifdef FLIGHT
        TRACKED_CONSTANT_SC(unsigned int, test_offset, 0);
    #else
        TRACKED_CONSTANT_SC(unsigned int, test_offset, 50000);
    #endif

    TRACKED_CONSTANT_SC(unsigned int, eeprom_controller_offset   ,   1);
    TRACKED_CONSTANT_SC(unsigned int, piksi_control_task_offset  ,   1100);
    TRACKED_CONSTANT_SC(unsigned int, adcs_monitor_offset        ,   7500);
    TRACKED_CONSTANT_SC(unsigned int, debug_task_offset          ,  140000 - 11251);
    TRACKED_CONSTANT_SC(unsigned int, attitude_estimator_offset  ,  35500 + test_offset);
    TRACKED_CONSTANT_SC(unsigned int, gomspace_controller_offset ,  56500 + test_offset);
    TRACKED_CONSTANT_SC(unsigned int, uplink_consumer_offset     ,  71500 + test_offset);
    TRACKED_CONSTANT_SC(unsigned int, mission_manager_offset     ,  71600 + test_offset);
    TRACKED_CONSTANT_SC(unsigned int, dcdc_controller_offset     ,  71700 + test_offset);
    TRACKED_CONSTANT_SC(unsigned int, attitude_controller_offset ,  71800 + test_offset);
    TRACKED_CONSTANT_SC(unsigned int, adcs_commander_offset      ,  71900 + test_offset);
    TRACKED_CONSTANT_SC(unsigned int, adcs_box_controller_offset ,  72000 + test_offset);
    TRACKED_CONSTANT_SC(unsigned int, orbit_controller_offset    ,  73000 + test_offset);
    TRACKED_CONSTANT_SC(unsigned int, prop_controller_offset     , 102000 + test_offset);
    TRACKED_CONSTANT_SC(unsigned int, docking_controller_offset  , 107000 + test_offset);
    TRACKED_CONSTANT_SC(unsigned int, downlink_producer_offset   , 109400 + test_offset);
    TRACKED_CONSTANT_SC(unsigned int, quake_manager_offset       , 109500 + test_offset);

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
