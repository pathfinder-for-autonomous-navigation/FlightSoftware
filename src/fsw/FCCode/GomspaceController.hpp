#pragma once

#ifndef GOMSPACE_CONTROLLER_HPP_
#define GOMSPACE_CONTROLLER_HPP_

#include "TimedControlTask.hpp"
#include "Drivers/Gomspace.hpp"
#include "Fault.hpp"

class GomspaceController : public TimedControlTask<void> {
   public:
    
    /**
     * @brief Construct a new Gomspace Controller object
     * 
     * @param registry 
     * @param offset
     * @param gs 
     */
    GomspaceController(StateFieldRegistry& registry, unsigned int offset,
        Devices::Gomspace &gs);

    /**
     * @brief Passes data from gomspace into statefields
     */
    void execute() override;

    /**
     * @brief Sets outputs from gomspace using statefield commands
     */
    void set_outputs();

   protected:
    Devices::Gomspace &gs;

    Fault get_hk_fault;
    
    Serializer<unsigned int> vboost_sr;
    ReadableStateField<unsigned int> vboost1_f;
    ReadableStateField<unsigned int> vboost2_f;
    ReadableStateField<unsigned int> vboost3_f;

    Serializer<unsigned int> vbatt_sr;
    ReadableStateField<unsigned int> vbatt_f;

    Serializer<unsigned int> curin_sr;
    ReadableStateField<unsigned int> curin1_f;
    ReadableStateField<unsigned int> curin2_f;
    ReadableStateField<unsigned int> curin3_f;

    Serializer<unsigned int> cursun_sr;
    ReadableStateField<unsigned int> cursun_f;

    Serializer<unsigned int> cursys_sr;
    ReadableStateField<unsigned int> cursys_f;

    Serializer<unsigned int> curout_sr;
    ReadableStateField<unsigned int> curout1_f;
    ReadableStateField<unsigned int> curout2_f;
    ReadableStateField<unsigned int> curout3_f;
    ReadableStateField<unsigned int> curout4_f;
    ReadableStateField<unsigned int> curout5_f;
    ReadableStateField<unsigned int> curout6_f;

    Serializer<bool> output_sr;
    ReadableStateField<bool> output1_f;
    ReadableStateField<bool> output2_f;
    ReadableStateField<bool> output3_f;
    ReadableStateField<bool> output4_f;
    ReadableStateField<bool> output5_f;
    ReadableStateField<bool> output6_f;

    Serializer<unsigned int> wdt_i2c_time_left_sr;
    ReadableStateField<unsigned int> wdt_i2c_time_left_f;

    Serializer<unsigned int> counter_wdt_i2c_sr;
    ReadableStateField<unsigned int> counter_wdt_i2c_f;

    Serializer<unsigned int> counter_boot_sr;
    ReadableStateField<unsigned int> counter_boot_f;

    Serializer<signed int> temp_sr;
    ReadableStateField<signed int> temp1_f;
    ReadableStateField<signed int> temp2_f;
    ReadableStateField<signed int> temp3_f;
    ReadableStateField<signed int> temp4_f;

    Serializer<unsigned char> bootcause_sr;
    ReadableStateField<unsigned char> bootcause_f;

    Serializer<unsigned char> battmode_sr;
    ReadableStateField<unsigned char> battmode_f;

    Serializer<unsigned char> pptmode_sr;
    ReadableStateField<unsigned char> pptmode_f;

    // The controller will set the outputs of the gomspace once a period (number of control cycles)
    unsigned int period = 300;

    // Command statefields to control the Gomspace outputs. Will
    // be set by various individual subsystems and the ground.
    Serializer<bool> power_cycle_outputs_cmd_sr;
    WritableStateField<bool> power_cycle_output1_cmd_f;
    WritableStateField<bool> power_cycle_output2_cmd_f;
    WritableStateField<bool> power_cycle_output3_cmd_f;
    WritableStateField<bool> power_cycle_output4_cmd_f;
    WritableStateField<bool> power_cycle_output5_cmd_f;
    WritableStateField<bool> power_cycle_output6_cmd_f;

    Serializer<unsigned int> pv_output_cmd_sr;
    WritableStateField<unsigned int> pv1_output_cmd_f;
    WritableStateField<unsigned int> pv2_output_cmd_f;
    WritableStateField<unsigned int> pv3_output_cmd_f;

    Serializer<unsigned char> ppt_mode_cmd_sr;
    WritableStateField<unsigned char> ppt_mode_cmd_f;

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
