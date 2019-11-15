#include "GomspaceController.hpp"

GomspaceController::GomspaceController(StateFieldRegistry &registry, unsigned int offset,
    Devices::Gomspace &_gs)
    : TimedControlTask<void>(registry, offset), gs(_gs), 
    vboost1_sr(0,10,10), vboost1_f("gomspace.vboost.output1", vboost1_sr),
    vboost2_sr(0,10,10), vboost2_f("gomspace.vboost.output2", vboost2_sr),
    vboost3_sr(0,10,10), vboost3_f("gomspace.vboost.output3", vboost3_sr),

    vbat_sr(0,10,10), vbat_f("gomspace.vbat", vbat_sr),

    curin1_sr(0,10,10), curin1_f("gomspace.curin.output1", curin1_sr),
    curin2_sr(0,10,10), curin2_f("gomspace.curin.output2", curin2_sr),
    curin3_sr(0,10,10), curin3_f("gomspace.curin.output3", curin3_sr),

    cursun_sr(0,10,10), cursun_f("gomspace.cursun", cursun_sr),
    cursys_sr(0,10,10), cursys_f("gomspace.cursys", cursys_sr),

    reserved1_sr(0,10,10), reserved1_f("gomspace.reserved1", reserved1_sr),

    curout1_sr(0,10,10), curout1_f("gomspace.curout.output1", curout1_sr),
    curout2_sr(0,10,10), curout2_f("gomspace.curout.output2", curout2_sr),
    curout3_sr(0,10,10), curout3_f("gomspace.curout.output3", curout3_sr),
    curout4_sr(0,10,10), curout4_f("gomspace.curout.output4", curout4_sr),
    curout5_sr(0,10,10), curout5_f("gomspace.curout.output5", curout5_sr),
    curout6_sr(0,10,10), curout6_f("gomspace.curout.output6", curout6_sr),

    output1_sr(0,10,10), output1_f("gomspace.output.output1", output1_sr),
    output2_sr(0,10,10), output2_f("gomspace.output.output2", output2_sr),
    output3_sr(0,10,10), output3_f("gomspace.output.output3", output3_sr),
    output4_sr(0,10,10), output4_f("gomspace.output.output4", output4_sr),
    output5_sr(0,10,10), output5_f("gomspace.output.output5", output5_sr),
    output6_sr(0,10,10), output6_f("gomspace.output.output6", output6_sr),
    output7_sr(0,10,10), output7_f("gomspace.output.output7", output7_sr),
    output8_sr(0,10,10), output8_f("gomspace.output.output8", output8_sr),

    output_on_delta1_sr(0,10,10), output_on_delta1_f("gomspace.output_on_delta.output1", output_on_delta1_sr),
    output_on_delta2_sr(0,10,10), output_on_delta2_f("gomspace.output_on_delta.output2", output_on_delta2_sr),
    output_on_delta3_sr(0,10,10), output_on_delta3_f("gomspace.output_on_delta.output3", output_on_delta3_sr),
    output_on_delta4_sr(0,10,10), output_on_delta4_f("gomspace.output_on_delta.output4", output_on_delta4_sr),
    output_on_delta5_sr(0,10,10), output_on_delta5_f("gomspace.output_on_delta.output5", output_on_delta5_sr),
    output_on_delta6_sr(0,10,10), output_on_delta6_f("gomspace.output_on_delta.output6", output_on_delta6_sr),
    output_on_delta7_sr(0,10,10), output_on_delta7_f("gomspace.output_on_delta.output7", output_on_delta7_sr),
    output_on_delta8_sr(0,10,10), output_on_delta8_f("gomspace.output_on_delta.output8", output_on_delta8_sr),

    output_off_delta1_sr(0,10,10), output_off_delta1_f("gomspace.output_off_delta.output1", output_off_delta1_sr),
    output_off_delta2_sr(0,10,10), output_off_delta2_f("gomspace.output_off_delta.output2", output_off_delta2_sr),
    output_off_delta3_sr(0,10,10), output_off_delta3_f("gomspace.output_off_delta.output3", output_off_delta3_sr),
    output_off_delta4_sr(0,10,10), output_off_delta4_f("gomspace.output_off_delta.output4", output_off_delta4_sr),
    output_off_delta5_sr(0,10,10), output_off_delta5_f("gomspace.output_off_delta.output5", output_off_delta5_sr),
    output_off_delta6_sr(0,10,10), output_off_delta6_f("gomspace.output_off_delta.output6", output_off_delta6_sr),
    output_off_delta7_sr(0,10,10), output_off_delta7_f("gomspace.output_off_delta.output7", output_off_delta7_sr),
    output_off_delta8_sr(0,10,10), output_off_delta8_f("gomspace.output_off_delta.output8", output_off_delta8_sr),

    latchup1_sr(0,10,10), latchup1_f("gomspace.latchup.output1", latchup1_sr),
    latchup2_sr(0,10,10), latchup2_f("gomspace.latchup.output2", latchup2_sr),
    latchup3_sr(0,10,10), latchup3_f("gomspace.latchup.output3", latchup3_sr),
    latchup4_sr(0,10,10), latchup4_f("gomspace.latchup.output4", latchup4_sr),
    latchup5_sr(0,10,10), latchup5_f("gomspace.latchup.output5", latchup5_sr),
    latchup6_sr(0,10,10), latchup6_f("gomspace.latchup.output6", latchup6_sr),

    wdt_i2c_time_left_sr(0,10,10), wdt_i2c_time_left_f("gomspace.wdt_i2c_time_left", wdt_i2c_time_left_sr),

    wdt_gnd_time_left_sr(0,10,10), wdt_gnd_time_left_f("gomspace.wdt_gnd_time_left", wdt_gnd_time_left_sr),

    wdt_csp_pings_left1_sr(0,10,10), wdt_csp_pings_left1_f("gomspace.wdt_csp_pings_left.output1", wdt_csp_pings_left1_sr),
    wdt_csp_pings_left2_sr(0,10,10), wdt_csp_pings_left2_f("gomspace.wdt_csp_pings_left.output2", wdt_csp_pings_left2_sr),

    counter_wdt_i2c_sr(0,10,10), counter_wdt_i2c_f("gomspace.counter_wdt_i2c", counter_wdt_i2c_sr),

    counter_wdt_gnd_sr(0,10,10), counter_wdt_gnd_f("gomspace.counter_wdt_gnd", counter_wdt_gnd_sr),

    counter_wdt_csp1_sr(0,10,10), counter_wdt_csp1_f("gomspace.counter_wdt_csp.output1", counter_wdt_csp1_sr),
    counter_wdt_csp2_sr(0,10,10), counter_wdt_csp2_f("gomspace.counter_wdt_csp.output2", counter_wdt_csp2_sr),

    counter_boot_sr(0,10,10), counter_boot_f("gomspace.counter_boot", counter_boot_sr),

    temp1_sr(0,10,10), temp1_f("gomspace.counter_boot.output1", temp1_sr),
    temp2_sr(0,10,10), temp2_f("gomspace.counter_boot.output2", temp2_sr),
    temp3_sr(0,10,10), temp3_f("gomspace.counter_boot.output3", temp3_sr),
    temp4_sr(0,10,10), temp4_f("gomspace.counter_boot.output4", temp4_sr),
    temp5_sr(0,10,10), temp5_f("gomspace.counter_boot.output5", temp5_sr),
    temp6_sr(0,10,10), temp6_f("gomspace.counter_boot.output6", temp6_sr),

    bootcause_sr(0,10,10), bootcause_f("gomspace.bootcause", bootcause_sr),

    battmode_sr(0,10,10), battmode_f("gomspace.battmode", battmode_sr),

    pptmode_sr(0,10,10), pptmode_f("gomspace.pptmode", pptmode_sr),

    reserved2_sr(0,10,10), reserved2_f("gomspace.reserved2", reserved2_sr)
    {
        add_readable_field(vboost1_f);
        add_readable_field(vboost2_f);
        add_readable_field(vboost3_f);

        add_readable_field(vbat_f);

        add_readable_field(curin1_f);
        add_readable_field(curin2_f);
        add_readable_field(curin3_f);

        add_readable_field(cursun_f);

        add_readable_field(cursys_f);

        add_readable_field(reserved1_f);

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

        add_readable_field(output_on_delta1_f);
        add_readable_field(output_on_delta2_f);
        add_readable_field(output_on_delta3_f);
        add_readable_field(output_on_delta4_f);
        add_readable_field(output_on_delta5_f);
        add_readable_field(output_on_delta6_f);
        add_readable_field(output_on_delta7_f);
        add_readable_field(output_on_delta8_f);

        add_readable_field(output_off_delta1_f);
        add_readable_field(output_off_delta2_f);
        add_readable_field(output_off_delta3_f);
        add_readable_field(output_off_delta4_f);
        add_readable_field(output_off_delta5_f);
        add_readable_field(output_off_delta6_f);
        add_readable_field(output_off_delta7_f);
        add_readable_field(output_off_delta8_f);

        add_readable_field(latchup1_f);
        add_readable_field(latchup2_f);
        add_readable_field(latchup3_f);
        add_readable_field(latchup4_f);
        add_readable_field(latchup5_f);
        add_readable_field(latchup6_f);

        add_readable_field(wdt_i2c_time_left_f);

        add_readable_field(wdt_gnd_time_left_f);

        add_readable_field(wdt_csp_pings_left1_f);
        add_readable_field(wdt_csp_pings_left2_f);

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

        add_readable_field(reserved2_f);
    }

void GomspaceController::execute() {
    //get hk data from struct in driver
    assert(gs.get_hk());
    //set statefields to respective data from 
    vboost1_f.set(gs.hk->vboost[0]);
    vboost2_f.set(gs.hk_data->vboost[1]);
    vboost3_f.set(gs.hk_data->vboost[2]);

    vbat_f.set(gs.hk_data.vbat);

    curin1_f.set(gs.hk_data.curin[0]);
    curin2_f.set(gs.hk_data.curin[1]);
    curin3_f.set(gs.hk_data.curin[2]);

    cursun_f.set(gs.hk_data.cursun);

    cursys_f.set(gs.hk_data.cursys);

    reserved1_f.set(gs.hk_data.reserved1);

    curout1_f.set(gs.hk_data->curout[0]);
    curout2_f.set(gs.hk_data.curout[1]);
    curout3_f.set(gs.hk_data.curout[2]);
    curout4_f.set(gs.hk_data.curout[3]);
    curout5_f.set(gs.hk_data.curout[4]);
    curout6_f.set(gs.hk_data.curout[5]);

    output1_f.set(gs.hk_data.output[0]);
    output2_f.set(gs.hk_data.output[1]);
    output3_f.set(gs.hk_data.output[2]);
    output4_f.set(gs.hk_data.output[3]);
    output5_f.set(gs.hk_data.output[4]);
    output6_f.set(gs.hk_data.output[5]);
    output7_f.set(gs.hk_data.output[6]);
    output8_f.set(gs.hk_data.output[7]);

    output_on_delta1_f.set(gs.hk_data.output_on_delta[0]);
    output_on_delta2_f.set(gs.hk_data.output_on_delta[1]);
    output_on_delta3_f.set(gs.hk_data.output_on_delta[2]);
    output_on_delta4_f.set(gs.hk_data.output_on_delta[3]);
    output_on_delta5_f.set(gs.hk_data.output_on_delta[4]);
    output_on_delta6_f.set(gs.hk_data.output_on_delta[5]);
    output_on_delta7_f.set(gs.hk_data.output_on_delta[6]);
    output_on_delta8_f.set(gs.hk_data.output_on_delta[7]);

    output_off_delta1_f.set(gs.hk_data.output_off_delta[0]);
    output_off_delta2_f.set(gs.hk_data.output_off_delta[1]);
    output_off_delta3_f.set(gs.hk_data.output_off_delta[2]);
    output_off_delta4_f.set(gs.hk_data.output_off_delta[3]);
    output_off_delta5_f.set(gs.hk_data.output_off_delta[4]);
    output_off_delta6_f.set(gs.hk_data.output_off_delta[5]);
    output_off_delta7_f.set(gs.hk_data.output_off_delta[6]);
    output_off_delta8_f.set(gs.hk_data.output_off_delta[7]);

    latchup1_f.set(gs.hk_data.latchup[0]);
    latchup2_f.set(gs.hk_data.latchup[1]);
    latchup3_f.set(gs.hk_data.latchup[2]);
    latchup4_f.set(gs.hk_data.latchup[3]);
    latchup5_f.set(gs.hk_data.latchup[4]);
    latchup6_f.set(gs.hk_data.latchup[5]);

    wdt_i2c_time_left_f.set(gs.hk_data.wdt_i2c_time_left);

    wdt_gnd_time_left_f.set(gs.hk_data.wdt_gnd_time_left);

    wdt_csp_pings_left1_f.set(gs.hk_data.wdt_csp_pings_left[0]);
    wdt_csp_pings_left2_f.set(gs.hk_data.wdt_csp_pings_left[1]);

    counter_wdt_i2c_f.set(gs.hk_data.counter_wdt_i2c);

    counter_wdt_gnd_f.set(gs.hk_data.counter_wdt_gnd);

    counter_wdt_csp1_f.set(gs.hk_data.counter_wdt_csp[0]);
    counter_wdt_csp2_f.set(gs.hk_data.counter_wdt_csp[1]);

    counter_boot_f.set(gs.hk_data.counter_boot);

    temp1_f.set(gs.hk_data.temp[0]);
    temp2_f.set(gs.hk_data.temp[1]);
    temp3_f.set(gs.hk_data.temp[2]);
    temp4_f.set(gs.hk_data.temp[3]);
    temp5_f.set(gs.hk_data.temp[4]);
    temp6_f.set(gs.hk_data.temp[5]);

    bootcause_f.set(gs.hk_data.bootcause);

    battmode_f.set(gs.hk_data.battmode);

    pptmode_f.set(gs.hk_data.pptmode);

    reserved2_f.set(gs.hk_data.reserved2);
}
