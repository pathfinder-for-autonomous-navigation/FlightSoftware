#include "GomspaceController.hpp"

GomspaceController::GomspaceController(StateFieldRegistry &registry, unsigned int offset,
    Devices::Gomspace &_gs)
    : TimedControlTask<void>(registry, "gomspace_rd", offset), gs(_gs), 

    // Serializer values taken from datasheet: https://drive.google.com/drive/u/0/folders/1IKB0q4r82Rd-L4oDUTEDPsAWOrWoveDo

    get_hk_fault("gomspace.get_hk", 1),
    low_batt_fault("gomspace.low_batt", 1),

    batt_threshold_sr(5000,9000,10),
    batt_threshold_f("gomspace.batt_threshold", batt_threshold_sr),

    vboost_sr(0,8500,6), // see pg 11
    vboost1_f("gomspace.vboost.output1", vboost_sr),
    vboost2_f("gomspace.vboost.output2", vboost_sr),
    vboost3_f("gomspace.vboost.output3", vboost_sr),

    vbatt_sr(5000,9500,10), // see pg 9
    vbatt_f("gomspace.vbatt", vbatt_sr),

    curin_sr(0,6000,10), 
    curin1_f("gomspace.curin.output1", curin_sr),
    curin2_f("gomspace.curin.output2", curin_sr),
    curin3_f("gomspace.curin.output3", curin_sr),

    cursun_sr(0,3000,10), 
    cursun_f("gomspace.cursun", cursun_sr),

    cursys_sr(0,12000,10), 
    cursys_f("gomspace.cursys", cursys_sr),

    curout_sr(0,12000,10), 
    curout1_f("gomspace.curout.output1", curout_sr),
    curout2_f("gomspace.curout.output2", curout_sr),
    curout3_f("gomspace.curout.output3", curout_sr),
    curout4_f("gomspace.curout.output4", curout_sr),
    curout5_f("gomspace.curout.output5", curout_sr),
    curout6_f("gomspace.curout.output6", curout_sr),

    output_sr(), 
    output1_f("gomspace.output.output1", output_sr),
    output2_f("gomspace.output.output2", output_sr),
    output3_f("gomspace.output.output3", output_sr),
    output4_f("gomspace.output.output4", output_sr),
    output5_f("gomspace.output.output5", output_sr),
    output6_f("gomspace.output.output6", output_sr),

    wdt_i2c_time_left_sr(99), 
    wdt_i2c_time_left_f("gomspace.wdt_i2c_time_left", wdt_i2c_time_left_sr),

    counter_wdt_i2c_sr(), 
    counter_wdt_i2c_f("gomspace.counter_wdt_i2c", counter_wdt_i2c_sr),

    counter_boot_sr(), 
    counter_boot_f("gomspace.counter_boot", counter_boot_sr),

    temp_sr(-40,125), 
    temp1_f("gomspace.temp.output1", temp_sr),
    temp2_f("gomspace.temp.output2", temp_sr),
    temp3_f("gomspace.temp.output3", temp_sr),
    temp4_f("gomspace.temp.output4", temp_sr),

    bootcause_sr(0,8), 
    bootcause_f("gomspace.bootcause", bootcause_sr),

    battmode_sr(0,4), 
    battmode_f("gomspace.battmode", battmode_sr),

    pptmode_sr(1,2), 
    pptmode_f("gomspace.pptmode", pptmode_sr),

    heater_sr(),
    heater_f("gomspace.heater", heater_sr),

    period_sr(),
    period_f("gomspace.powercycle_period", period_sr),

    power_cycle_outputs_cmd_sr(),
    power_cycle_output1_cmd_f("gomspace.power_cycle_output1_cmd", power_cycle_outputs_cmd_sr),
    power_cycle_output2_cmd_f("gomspace.power_cycle_output2_cmd", power_cycle_outputs_cmd_sr),
    power_cycle_output3_cmd_f("gomspace.power_cycle_output3_cmd", power_cycle_outputs_cmd_sr),
    power_cycle_output4_cmd_f("gomspace.power_cycle_output4_cmd", power_cycle_outputs_cmd_sr),
    power_cycle_output5_cmd_f("gomspace.power_cycle_output5_cmd", power_cycle_outputs_cmd_sr),
    power_cycle_output6_cmd_f("gomspace.power_cycle_output6_cmd", power_cycle_outputs_cmd_sr),

    pv_output_cmd_sr(0,8500,9), // see pg 11
    pv1_output_cmd_f("gomspace.pv1_cmd", pv_output_cmd_sr),
    pv2_output_cmd_f("gomspace.pv2_cmd", pv_output_cmd_sr),
    pv3_output_cmd_f("gomspace.pv3_cmd", pv_output_cmd_sr),

    ppt_mode_cmd_sr(1,2),
    ppt_mode_cmd_f("gomspace.pptmode_cmd", ppt_mode_cmd_sr),

    heater_cmd_sr(),
    heater_cmd_f("gomspace.heater_cmd", heater_cmd_sr),

    counter_reset_cmd_sr(),
    counter_reset_cmd_f("gomspace.counter_reset_cmd", counter_reset_cmd_sr),

    gs_reset_cmd_sr(),
    gs_reset_cmd_f("gomspace.gs_reset_cmd", gs_reset_cmd_sr),

    gs_reboot_cmd_sr(),
    gs_reboot_cmd_f("gomspace.gs_reboot_cmd", gs_reboot_cmd_sr),

    piksi_off_sr(),
    piksi_off_f("gomspace.piksi_off", piksi_off_sr)
    {
        add_fault(get_hk_fault);
        add_fault(low_batt_fault);

        add_writable_field(batt_threshold_f);
        batt_threshold_f.set(7300);
        
        add_readable_field(vboost1_f);
        add_readable_field(vboost2_f);
        add_readable_field(vboost3_f);

        add_readable_field(vbatt_f);

        add_readable_field(curin1_f);
        add_readable_field(curin2_f);
        add_readable_field(curin3_f);

        add_readable_field(cursun_f);

        add_readable_field(cursys_f);

        add_readable_field(curout1_f);
        add_readable_field(curout2_f);
        add_readable_field(curout3_f);
        add_readable_field(curout4_f);
        add_readable_field(curout5_f);
        add_readable_field(curout6_f);

        add_readable_field(output1_f);
        add_readable_field(output2_f);
        add_readable_field(output3_f);
        add_readable_field(output4_f);
        add_readable_field(output5_f);
        add_readable_field(output6_f);

        add_readable_field(wdt_i2c_time_left_f);

        add_readable_field(counter_wdt_i2c_f);

        add_readable_field(counter_boot_f);

        add_readable_field(temp1_f);
        add_readable_field(temp2_f);
        add_readable_field(temp3_f);
        add_readable_field(temp4_f);

        add_readable_field(bootcause_f);

        add_readable_field(battmode_f);

        add_readable_field(pptmode_f);

        add_readable_field(heater_f);

        // If PAN::one_day_ccno is very short due to the SPEEDUP flag, ensure the period is positive
        // to prevent a divide-by-zero error.
        add_writable_field(period_f);
        period_f.set(thirty_seconds_ccno > 0 ? thirty_seconds_ccno : 1);
        
        add_writable_field(power_cycle_output1_cmd_f);
        add_writable_field(power_cycle_output2_cmd_f);
        add_writable_field(power_cycle_output3_cmd_f);
        add_writable_field(power_cycle_output4_cmd_f);
        add_writable_field(power_cycle_output5_cmd_f);
        add_writable_field(power_cycle_output6_cmd_f);

        add_writable_field(pv1_output_cmd_f);
        add_writable_field(pv2_output_cmd_f);
        add_writable_field(pv3_output_cmd_f);
        pv1_output_cmd_f.set(default_pv_cmd);
        pv2_output_cmd_f.set(default_pv_cmd);
        pv3_output_cmd_f.set(default_pv_cmd);

        add_writable_field(ppt_mode_cmd_f);
        ppt_mode_cmd_f.set(default_ppt_mode);

        add_writable_field(heater_cmd_f);

        add_writable_field(counter_reset_cmd_f);

        add_writable_field(gs_reset_cmd_f);

        add_writable_field(gs_reboot_cmd_f);

        add_writable_field(piksi_off_f);
        piksi_off_f.set(true);
     }

void GomspaceController::execute() {


    //Check that we can get hk data
    get_hk_fault.evaluate(!gs.get_hk());

    // Check that the battery voltage is above the threshold
    low_batt_fault.evaluate(vbatt_f.get() < batt_threshold_f.get());

    // On the first control cycle, set the command statefields to the current values 
    // in the hk struct to prevent unwanted writes.
    if (control_cycle_count==1){
        power_cycle_output1_cmd_f.set(false);
        power_cycle_output2_cmd_f.set(false);
        power_cycle_output3_cmd_f.set(false);
        power_cycle_output4_cmd_f.set(false);
        power_cycle_output5_cmd_f.set(false);
        power_cycle_output6_cmd_f.set(false);

        pv1_output_cmd_f.set(gs.hk->vboost[0]);
        pv2_output_cmd_f.set(gs.hk->vboost[1]);
        pv3_output_cmd_f.set(gs.hk->vboost[2]);

        heater_cmd_f.set(gs.get_heater());

        counter_reset_cmd_f.set(false);
        gs_reset_cmd_f.set(false);
        gs_reboot_cmd_f.set(false);
    }

    // Set the gomspace outputs to the values of the statefield commands around every 30 seconds
    if (control_cycle_count%period_f.get()==0){
        power_cycle_outputs();
    }

    if(pptmode_f.get() == 2){ // 2 is fixed ppt
        // Set power voltage command
        if (vboost1_f.get()!=pv1_output_cmd_f.get() || vboost2_f.get()!=pv2_output_cmd_f.get() || vboost3_f.get()!=pv3_output_cmd_f.get()) {
            gs.set_pv_volt(pv1_output_cmd_f.get(), pv2_output_cmd_f.get(), pv3_output_cmd_f.get());
        }
    }

    // Set PPT mode command
    if (pptmode_f.get()!=ppt_mode_cmd_f.get()){
        gs.set_ppt_mode(ppt_mode_cmd_f.get());
    }

    // Turn on/off the heater command
    if (heater_cmd_f.get()==true && gs.get_heater() == 0) {
        gs.turn_on_heater();
    }
    else if (heater_cmd_f.get()==false && gs.get_heater() == 1){
        gs.turn_off_heater();
    }

    // Reset commands
    if (counter_reset_cmd_f.get()==true) {
        gs.reset_counters();
        counter_reset_cmd_f.set(false);
    }

    if (gs_reset_cmd_f.get()==true) {
        gs.hard_reset();
        gs_reset_cmd_f.set(false);
    }

    if (gs_reboot_cmd_f.get()==true) {
        gs.reboot();
        gs_reboot_cmd_f.set(false);
    }

    //set data-in statefields to respective data from hk struct 
    vboost1_f.set(gs.hk->vboost[0]);
    vboost2_f.set(gs.hk->vboost[1]);
    vboost3_f.set(gs.hk->vboost[2]);

    vbatt_f.set(gs.hk->vbatt);

    curin1_f.set(gs.hk->curin[0]);
    curin2_f.set(gs.hk->curin[1]);
    curin3_f.set(gs.hk->curin[2]);

    cursun_f.set(gs.hk->cursun);

    cursys_f.set(gs.hk->cursys);

    curout1_f.set(gs.hk->curout[0]);
    curout2_f.set(gs.hk->curout[1]);
    curout3_f.set(gs.hk->curout[2]);
    curout4_f.set(gs.hk->curout[3]);
    curout5_f.set(gs.hk->curout[4]);
    curout6_f.set(gs.hk->curout[5]);

    output1_f.set(gs.hk->output[0]);
    output2_f.set(gs.hk->output[1]);
    output3_f.set(gs.hk->output[2]);
    output4_f.set(gs.hk->output[3]);
    output5_f.set(gs.hk->output[4]);
    output6_f.set(gs.hk->output[5]);

    wdt_i2c_time_left_f.set(gs.hk->wdt_i2c_time_left);

    counter_wdt_i2c_f.set(gs.hk->counter_wdt_i2c);

    counter_boot_f.set(gs.hk->counter_boot);

    temp1_f.set(gs.hk->temp[0]);
    temp2_f.set(gs.hk->temp[1]);
    temp3_f.set(gs.hk->temp[2]);
    temp4_f.set(gs.hk->temp[3]);

    bootcause_f.set(gs.hk->bootcause);

    battmode_f.set(gs.hk->battmode);

    pptmode_f.set(gs.hk->pptmode);

    heater_f.set(gs.get_heater()==1);

    // if wait < deployment_wait, turn off power to piksi
    if(piksi_off_f.get()){
        gs.set_single_output(0,0); // (output port = OUT-1, 0 for off)
    }

}

void GomspaceController::power_cycle_outputs() {
    auto powercycle_logic = [&]
        (WritableStateField<bool>& cmd_f,
         ReadableStateField<bool>& output_f,
         int idx)
    {
        if (cmd_f.get()) {

            // TODO add powercycling event
            if (output_f.get()) {
                gs.set_single_output(idx,0);
            }
            else {
                gs.set_single_output(idx,1);
                cmd_f.set(false);
            }
        }
    };

    // Power cycle output channels
    if(!piksi_off_f.get()){
        powercycle_logic(power_cycle_output1_cmd_f, output1_f, 0);

    }
    powercycle_logic(power_cycle_output2_cmd_f, output2_f, 1);
    powercycle_logic(power_cycle_output3_cmd_f, output3_f, 2);
    powercycle_logic(power_cycle_output4_cmd_f, output4_f, 3);
    powercycle_logic(power_cycle_output5_cmd_f, output5_f, 4);
    powercycle_logic(power_cycle_output6_cmd_f, output6_f, 5);
}
