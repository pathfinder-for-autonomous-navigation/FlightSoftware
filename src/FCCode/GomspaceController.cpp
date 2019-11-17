#include "GomspaceController.hpp"

GomspaceController::GomspaceController(StateFieldRegistry &registry, unsigned int offset,
    Devices::Gomspace &_gs)
    : TimedControlTask<void>(registry, offset), gs(_gs), 
    vboost_sr(0,3700,12), 
    vboost1_f("gomspace.vboost.output1", vboost_sr),
    vboost2_f("gomspace.vboost.output2", vboost_sr),
    vboost3_f("gomspace.vboost.output3", vboost_sr),

    vbatt_sr(5000,9000,14), vbatt_f("gomspace.vbatt", vbatt_sr),

    curin_sr(0,10,10), 
    curin1_f("gomspace.curin.output1", curin_sr),
    curin2_f("gomspace.curin.output2", curin_sr),
    curin3_f("gomspace.curin.output3", curin_sr),

    cursun_sr(0,10,10), cursun_f("gomspace.cursun", cursun_sr),

    cursys_sr(0,10,10), cursys_f("gomspace.cursys", cursys_sr),

    curout_sr(0,10,10), 
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
    output7_f("gomspace.output.output7", output_sr),
    output8_f("gomspace.output.output8", output_sr),

    wdt_i2c_time_left_sr(0,1000,10), wdt_i2c_time_left_f("gomspace.wdt_i2c_time_left", wdt_i2c_time_left_sr),

    counter_wdt_i2c_sr(), counter_wdt_i2c_f("gomspace.counter_wdt_i2c", counter_wdt_i2c_sr),

    counter_wdt_gnd_sr(0,10,10), counter_wdt_gnd_f("gomspace.counter_wdt_gnd", counter_wdt_gnd_sr),

    counter_wdt_csp_sr(0,10,10), 
    counter_wdt_csp1_f("gomspace.counter_wdt_csp.output1", counter_wdt_csp_sr),
    counter_wdt_csp2_f("gomspace.counter_wdt_csp.output2", counter_wdt_csp_sr),

    counter_boot_sr(0,10,10), counter_boot_f("gomspace.counter_boot", counter_boot_sr),

    temp_sr(-40,125), 
    temp1_f("gomspace.temp.output1", temp_sr),
    temp2_f("gomspace.temp.output2", temp_sr),
    temp3_f("gomspace.temp.output3", temp_sr),
    temp4_f("gomspace.temp.output4", temp_sr),
    temp5_f("gomspace.temp.output5", temp_sr),
    temp6_f("gomspace.temp.output6", temp_sr),

    bootcause_sr(0,8), bootcause_f("gomspace.bootcause", bootcause_sr),

    battmode_sr(0,10,10), battmode_f("gomspace.battmode", battmode_sr),

    pptmode_sr(0,10,10), pptmode_f("gomspace.pptmode", pptmode_sr)
    {
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
        add_readable_field(output7_f);
        add_readable_field(output8_f);

        add_readable_field(wdt_i2c_time_left_f);

        add_readable_field(counter_wdt_i2c_f);

        add_readable_field(counter_wdt_gnd_f);

        add_readable_field(counter_wdt_csp1_f);
        add_readable_field(counter_wdt_csp2_f);

        add_readable_field(counter_boot_f);

        add_readable_field(temp1_f);
        add_readable_field(temp2_f);
        add_readable_field(temp3_f);
        add_readable_field(temp4_f);
        add_readable_field(temp5_f);
        add_readable_field(temp6_f);

        add_readable_field(bootcause_f);

        add_readable_field(battmode_f);

        add_readable_field(pptmode_f);
     }

void GomspaceController::execute() {
    //get hk data from struct in driver
    assert(gs.get_hk());
    //set statefields to respective data from hk struct 
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
    output7_f.set(gs.hk->output[6]);
    output8_f.set(gs.hk->output[7]);

    wdt_i2c_time_left_f.set(gs.hk->wdt_i2c_time_left);

    counter_wdt_i2c_f.set(gs.hk->counter_wdt_i2c);

    counter_wdt_gnd_f.set(gs.hk->counter_wdt_gnd);

    counter_wdt_csp1_f.set(gs.hk->counter_wdt_csp[0]);
    counter_wdt_csp2_f.set(gs.hk->counter_wdt_csp[1]);

    counter_boot_f.set(gs.hk->counter_boot);

    temp1_f.set(gs.hk->temp[0]);
    temp2_f.set(gs.hk->temp[1]);
    temp3_f.set(gs.hk->temp[2]);
    temp4_f.set(gs.hk->temp[3]);
    temp5_f.set(gs.hk->temp[4]);
    temp6_f.set(gs.hk->temp[5]);

    bootcause_f.set(gs.hk->bootcause);

    battmode_f.set(gs.hk->battmode);

    pptmode_f.set(gs.hk->pptmode);
}
