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
#include "Estimators.hpp"
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

    Estimators estimators;

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

    ReadableStateField<unsigned int> control_cycle_duration_f;
    sys_time_t prev_sys_time;
    
    OrbitController orbit_controller;
    PropController prop_controller;
    MissionManager mission_manager;
    AttitudeController attitude_controller; // needs adcs.state from MissionManager
    ADCSCommander adcs_commander; // will need inputs from computer++
    ADCSBoxController adcs_box_controller; // needs adcs.state from MissionManager

   public:
    /*
     * @brief Construct a new Main Control Loop Task object
     * 
     * @param registry State field registry
     * @param flow_data Metadata for telemetry flows.
     */
    MainControlLoop(StateFieldRegistry& registry,
        const std::vector<DownlinkProducer::FlowData>& flow_data, 
        const std::vector<DownlinkProducer::FlowData>& startup_flows,
        const std::vector<DownlinkProducer::FlowData>& detumble_flows,
        const std::vector<DownlinkProducer::FlowData>& close_approach_flows,
        const std::vector<DownlinkProducer::FlowData>& docking_docked_flows);

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
