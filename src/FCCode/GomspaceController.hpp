#pragma once

#ifndef GOMSPACE_CONTROLLER_HPP_
#define GOMSPACE_CONTROLLER_HPP_

#include <TimedControlTask.hpp>
#include <Gomspace.hpp>

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
     * @brief sets outputs from gomspace to statefields
     */
    void execute() override;

   protected:
    Devices::Gomspace &gs;
    
    Serializer<unsigned int> vboost1_sr;
    ReadableStateField<unsigned int> vboost1_f;

    Serializer<unsigned int> vboost2_sr;
    ReadableStateField<unsigned int> vboost2_f;

    Serializer<unsigned int> vboost3_sr;
    ReadableStateField<unsigned int> vboost3_f;

    Serializer<int> vbatt_sr;
    ReadableStateField<int> vbatt_f;

    // Serializer<unsigned int> curin1_sr;
    // ReadableStateField<unsigned int> curin1_f;

    // Serializer<unsigned int> curin2_sr;
    // ReadableStateField<unsigned int> curin2_f;

    // Serializer<unsigned int> curin3_sr;
    // ReadableStateField<unsigned int> curin3_f;

    // Serializer<unsigned int> cursun_sr;
    // ReadableStateField<unsigned int> cursun_f;

    // Serializer<unsigned int> cursys_sr;
    // ReadableStateField<unsigned int> cursys_f;

    // Serializer<unsigned int> reserved1_sr;
    // ReadableStateField<unsigned int> reserved1_f;

    // Serializer<unsigned int> curout1_sr;
    // ReadableStateField<unsigned int> curout1_f;

    // Serializer<unsigned int> curout2_sr;
    // ReadableStateField<unsigned int> curout2_f;

    // Serializer<unsigned int> curout3_sr;
    // ReadableStateField<unsigned int> curout3_f;

    // Serializer<unsigned int> curout4_sr;
    // ReadableStateField<unsigned int> curout4_f;

    // Serializer<unsigned int> curout5_sr;
    // ReadableStateField<unsigned int> curout5_f;

    // Serializer<unsigned int> curout6_sr;
    // ReadableStateField<unsigned int> curout6_f;

    // Serializer<unsigned int> output1_sr;
    // ReadableStateField<unsigned int> output1_f;

    // Serializer<unsigned int> output2_sr;
    // ReadableStateField<unsigned int> output2_f;

    // Serializer<unsigned int> output3_sr;
    // ReadableStateField<unsigned int> output3_f;

    // Serializer<unsigned int> output4_sr;
    // ReadableStateField<unsigned int> output4_f;

    // Serializer<unsigned int> output5_sr;
    // ReadableStateField<unsigned int> output5_f;

    // Serializer<unsigned int> output6_sr;
    // ReadableStateField<unsigned int> output6_f;

    // Serializer<unsigned int> output7_sr;
    // ReadableStateField<unsigned int> output7_f;

    // Serializer<unsigned int> output8_sr;
    // ReadableStateField<unsigned int> output8_f;

    // Serializer<unsigned int> output_on_delta1_sr;
    // ReadableStateField<unsigned int> output_on_delta1_f;

    // Serializer<unsigned int> output_on_delta2_sr;
    // ReadableStateField<unsigned int> output_on_delta2_f;

    // Serializer<unsigned int> output_on_delta3_sr;
    // ReadableStateField<unsigned int> output_on_delta3_f;

    // Serializer<unsigned int> output_on_delta4_sr;
    // ReadableStateField<unsigned int> output_on_delta4_f;

    // Serializer<unsigned int> output_on_delta5_sr;
    // ReadableStateField<unsigned int> output_on_delta5_f;

    // Serializer<unsigned int> output_on_delta6_sr;
    // ReadableStateField<unsigned int> output_on_delta6_f;

    // Serializer<unsigned int> output_on_delta7_sr;
    // ReadableStateField<unsigned int> output_on_delta7_f;

    // Serializer<unsigned int> output_on_delta8_sr;
    // ReadableStateField<unsigned int> output_on_delta8_f;

    // Serializer<unsigned int> output_off_delta1_sr;
    // ReadableStateField<unsigned int> output_off_delta1_f;

    // Serializer<unsigned int> output_off_delta2_sr;
    // ReadableStateField<unsigned int> output_off_delta2_f;

    // Serializer<unsigned int> output_off_delta3_sr;
    // ReadableStateField<unsigned int> output_off_delta3_f;

    // Serializer<unsigned int> output_off_delta4_sr;
    // ReadableStateField<unsigned int> output_off_delta4_f;

    // Serializer<unsigned int> output_off_delta5_sr;
    // ReadableStateField<unsigned int> output_off_delta5_f;

    // Serializer<unsigned int> output_off_delta6_sr;
    // ReadableStateField<unsigned int> output_off_delta6_f;

    // Serializer<unsigned int> output_off_delta7_sr;
    // ReadableStateField<unsigned int> output_off_delta7_f;

    // Serializer<unsigned int> output_off_delta8_sr;
    // ReadableStateField<unsigned int> output_off_delta8_f;

    // Serializer<unsigned int> latchup1_sr;
    // ReadableStateField<unsigned int> latchup1_f;

    // Serializer<unsigned int> latchup2_sr;
    // ReadableStateField<unsigned int> latchup2_f;

    // Serializer<unsigned int> latchup3_sr;
    // ReadableStateField<unsigned int> latchup3_f;

    // Serializer<unsigned int> latchup4_sr;
    // ReadableStateField<unsigned int> latchup4_f;

    // Serializer<unsigned int> latchup5_sr;
    // ReadableStateField<unsigned int> latchup5_f;

    // Serializer<unsigned int> latchup6_sr;
    // ReadableStateField<unsigned int> latchup6_f;

    // Serializer<unsigned int> wdt_i2c_time_left_sr;
    // ReadableStateField<unsigned int> wdt_i2c_time_left_f;

    // Serializer<unsigned int> wdt_gnd_time_left_sr;
    // ReadableStateField<unsigned int> wdt_gnd_time_left_f;

    // Serializer<unsigned int> wdt_csp_pings_left1_sr;
    // ReadableStateField<unsigned int> wdt_csp_pings_left1_f;

    // Serializer<unsigned int> wdt_csp_pings_left2_sr;
    // ReadableStateField<unsigned int> wdt_csp_pings_left2_f;

    // Serializer<unsigned int> counter_wdt_i2c_sr;
    // ReadableStateField<unsigned int> counter_wdt_i2c_f;

    // Serializer<unsigned int> counter_wdt_gnd_sr;
    // ReadableStateField<unsigned int> counter_wdt_gnd_f;

    // Serializer<unsigned int> counter_wdt_csp1_sr;
    // ReadableStateField<unsigned int> counter_wdt_csp1_f;

    // Serializer<unsigned int> counter_wdt_csp2_sr;
    // ReadableStateField<unsigned int> counter_wdt_csp2_f;

    // Serializer<unsigned int> counter_boot_sr;
    // ReadableStateField<unsigned int> counter_boot_f;

    // Serializer<unsigned int> temp1_sr;
    // ReadableStateField<unsigned int> temp1_f;

    // Serializer<unsigned int> temp2_sr;
    // ReadableStateField<unsigned int> temp2_f;

    // Serializer<unsigned int> temp3_sr;
    // ReadableStateField<unsigned int> temp3_f;

    // Serializer<unsigned int> temp4_sr;
    // ReadableStateField<unsigned int> temp4_f;

    // Serializer<unsigned int> temp5_sr;
    // ReadableStateField<unsigned int> temp5_f;

    // Serializer<unsigned int> temp6_sr;
    // ReadableStateField<unsigned int> temp6_f;

    // Serializer<unsigned int> bootcause_sr;
    // ReadableStateField<unsigned int> bootcause_f;

    // Serializer<unsigned int> battmode_sr;
    // ReadableStateField<unsigned int> battmode_f;

    // Serializer<unsigned int> pptmode_sr;
    // ReadableStateField<unsigned int> pptmode_f;

    // Serializer<unsigned int> reserved2_sr;
    // ReadableStateField<unsigned int> reserved2_f;

};

#endif
