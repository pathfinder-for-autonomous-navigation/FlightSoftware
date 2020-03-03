#pragma once

#ifndef GOMSPACE_CONTROLLER_HPP_
#define GOMSPACE_CONTROLLER_HPP_

#include "TimedControlTask.hpp"
#include "Drivers/Gomspace.hpp"

class GomspaceController : public TimedControlTask<void> {
   public:
    
    /**
     * @brief Construct a new Gomspace Controller object
     * 
     * @param registry 
     * @param offset
     * @param gs 
     */
    GomspaceController(StateFieldRegistry& registry, uint32_t offset,
        Devices::Gomspace &gs);

    /**
     * @brief Passes data from gomspace into statefields
     */
    void execute() override;

    /**
     * @brief Power cycles outputs from gomspace using statefield commands
     */
    void power_cycle_outputs();

   protected:
    Devices::Gomspace &gs;

    Fault get_hk_fault;
    Fault low_batt_fault;

    Serializer<uint32_t> batt_threshold_sr;
    WritableStateField<uint32_t> batt_threshold_f;
    
    Serializer<uint32_t> vboost_sr;
    ReadableStateField<uint32_t> vboost1_f;
    ReadableStateField<uint32_t> vboost2_f;
    ReadableStateField<uint32_t> vboost3_f;

    Serializer<uint32_t> vbatt_sr;
    ReadableStateField<uint32_t> vbatt_f;

    Serializer<uint32_t> curin_sr;
    ReadableStateField<uint32_t> curin1_f;
    ReadableStateField<uint32_t> curin2_f;
    ReadableStateField<uint32_t> curin3_f;

    Serializer<uint32_t> cursun_sr;
    ReadableStateField<uint32_t> cursun_f;

    Serializer<uint32_t> cursys_sr;
    ReadableStateField<uint32_t> cursys_f;

    Serializer<uint32_t> curout_sr;
    ReadableStateField<uint32_t> curout1_f;
    ReadableStateField<uint32_t> curout2_f;
    ReadableStateField<uint32_t> curout3_f;
    ReadableStateField<uint32_t> curout4_f;
    ReadableStateField<uint32_t> curout5_f;
    ReadableStateField<uint32_t> curout6_f;

    Serializer<bool> output_sr;
    ReadableStateField<bool> output1_f;
    ReadableStateField<bool> output2_f;
    ReadableStateField<bool> output3_f;
    ReadableStateField<bool> output4_f;
    ReadableStateField<bool> output5_f;
    ReadableStateField<bool> output6_f;

    Serializer<uint32_t> wdt_i2c_time_left_sr;
    ReadableStateField<uint32_t> wdt_i2c_time_left_f;

    Serializer<uint32_t> counter_wdt_i2c_sr;
    ReadableStateField<uint32_t> counter_wdt_i2c_f;

    Serializer<uint32_t> counter_boot_sr;
    ReadableStateField<uint32_t> counter_boot_f;

    Serializer<int32_t> temp_sr;
    ReadableStateField<int32_t> temp1_f;
    ReadableStateField<int32_t> temp2_f;
    ReadableStateField<int32_t> temp3_f;
    ReadableStateField<int32_t> temp4_f;

    Serializer<uint8_t> bootcause_sr;
    ReadableStateField<uint8_t> bootcause_f;

    Serializer<uint8_t> battmode_sr;
    ReadableStateField<uint8_t> battmode_f;

    Serializer<uint8_t> pptmode_sr;
    ReadableStateField<uint8_t> pptmode_f;

    Serializer<bool> heater_sr;
    ReadableStateField<bool> heater_f;

    // The controller will set the outputs of the gomspace once a period (number of control cycles)
    uint32_t period = 300;

    // Command statefields to control the Gomspace outputs. Will
    // be set by various individual subsystems and the ground.
    Serializer<bool> power_cycle_outputs_cmd_sr;
    WritableStateField<bool> power_cycle_output1_cmd_f;
    WritableStateField<bool> power_cycle_output2_cmd_f;
    WritableStateField<bool> power_cycle_output3_cmd_f;
    WritableStateField<bool> power_cycle_output4_cmd_f;
    WritableStateField<bool> power_cycle_output5_cmd_f;
    WritableStateField<bool> power_cycle_output6_cmd_f;

    Serializer<uint32_t> pv_output_cmd_sr;
    WritableStateField<uint32_t> pv1_output_cmd_f;
    WritableStateField<uint32_t> pv2_output_cmd_f;
    WritableStateField<uint32_t> pv3_output_cmd_f;

    Serializer<uint8_t> ppt_mode_cmd_sr;
    WritableStateField<uint8_t> ppt_mode_cmd_f;

    Serializer<bool> heater_cmd_sr;
    WritableStateField<bool> heater_cmd_f;

    Serializer<bool> counter_reset_cmd_sr;
    WritableStateField<bool> counter_reset_cmd_f;

    Serializer<bool> gs_reset_cmd_sr;
    WritableStateField<bool> gs_reset_cmd_f;

    Serializer<bool> gs_reboot_cmd_sr;
    WritableStateField<bool> gs_reboot_cmd_f;

};

#endif
