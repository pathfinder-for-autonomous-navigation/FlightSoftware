#pragma once

#ifndef GOMSPACE_CONTROLLER_HPP_
#define GOMSPACE_CONTROLLER_HPP_

#include "../../test/StateFieldRegistryMock.hpp"
#include <TimedControlTask.hpp>
#include "../../lib/Drivers/Gomspace.hpp"

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
     * @brief 
     */
    void execute() override;

   protected:
    //Devices::Gomspace::eps_hk_t hk_data;
    Devices::Gomspace::eps_config_t config_data;
    Devices::Gomspace::eps_config2_t config2_data;
    Devices::Gomspace &gs(&hk_data, &config_data, &config2_data);

    Devices::Gomspace &gs;
    
    //how do I serialize arrays?
    Serializer<unsigned int> vboost1_sr;
    ReadableStateField<unsigned int> vboost1_f;

    Serializer<unsigned int> vboost2_sr;
    ReadableStateField<unsigned int> vboost2_f;

    Serializer<unsigned int> vboost3_sr;
    ReadableStateField<unsigned int> vboost3_f;

    //is it okay if I use int instead of short int?
    Serializer<unsigned int> vbat_sr;
    ReadableStateField<unsigned int> vbat_f;

    Serializer<unsigned int> curin1_sr;
    ReadableStateField<unsigned int> curin1_f;

    Serializer<unsigned int> curin2_sr;
    ReadableStateField<unsigned int> curin2_f;

    Serializer<unsigned int> curin3_sr;
    ReadableStateField<unsigned int> curin3_f;

    Serializer<unsigned int> cursun_sr;
    ReadableStateField<unsigned int> cursun_f;

    Serializer<unsigned int> cursys_sr;
    ReadableStateField<unsigned int> cursys_f;

    Serializer<unsigned int> reserved1_sr;
    ReadableStateField<unsigned int> reserved1_f;

    Serializer<unsigned int> curout1_sr;
    ReadableStateField<unsigned int> curout1_f;

    Serializer<unsigned int> curout2_sr;
    ReadableStateField<unsigned int> curout2_f;

    Serializer<unsigned int> curout3_sr;
    ReadableStateField<unsigned int> curout3_f;

    Serializer<unsigned int> curout4_sr;
    ReadableStateField<unsigned int> curout4_f;

    Serializer<unsigned int> curout5_sr;
    ReadableStateField<unsigned int> curout5_f;

    Serializer<unsigned int> curout6_sr;
    ReadableStateField<unsigned int> curout6_f;

    //how do i serialize chars?
    Serializer<char> output1_sr;
    ReadableStateField<char> output1_f;

    Serializer<char> output2_sr;
    ReadableStateField<char> output2_f;

    Serializer<char> output3_sr;
    ReadableStateField<char> output3_f;

    Serializer<char> output4_sr;
    ReadableStateField<char> output4_f;

    Serializer<char> output5_sr;
    ReadableStateField<char> output5_f;

    Serializer<char> output6_sr;
    ReadableStateField<char> output6_f;

    Serializer<char> output7_sr;
    ReadableStateField<char> output7_f;

    Serializer<char> output8_sr;
    ReadableStateField<char> output8_f;

    Serializer<unsigned int> output_on_delta1_sr;
    ReadableStateField<unsigned int> output_on_delta1_f;

    Serializer<unsigned int> output_on_delta2_sr;
    ReadableStateField<unsigned int> output_on_delta2_f;

    Serializer<unsigned int> output_on_delta3_sr;
    ReadableStateField<unsigned int> output_on_delta3_f;

    Serializer<unsigned int> output_on_delta4_sr;
    ReadableStateField<unsigned int> output_on_delta4_f;

    Serializer<unsigned int> output_on_delta5_sr;
    ReadableStateField<unsigned int> output_on_delta5_f;

    Serializer<unsigned int> output_on_delta6_sr;
    ReadableStateField<unsigned int> output_on_delta6_f;

    Serializer<unsigned int> output_on_delta7_sr;
    ReadableStateField<unsigned int> output_on_delta7_f;

    Serializer<unsigned int> output_on_delta8_sr;
    ReadableStateField<unsigned int> output_on_delta8_f;

    Serializer<unsigned int> output_off_delta1_sr;
    ReadableStateField<unsigned int> output_off_delta1_f;

    Serializer<unsigned int> output_off_delta2_sr;
    ReadableStateField<unsigned int> output_off_delta2_f;

    Serializer<unsigned int> output_off_delta3_sr;
    ReadableStateField<unsigned int> output_off_delta3_f;

    Serializer<unsigned int> output_off_delta4_sr;
    ReadableStateField<unsigned int> output_off_delta4_f;

    Serializer<unsigned int> output_off_delta5_sr;
    ReadableStateField<unsigned int> output_off_delta5_f;

    Serializer<unsigned int> output_off_delta6_sr;
    ReadableStateField<unsigned int> output_off_delta6_f;

    Serializer<unsigned int> output_off_delta7_sr;
    ReadableStateField<unsigned int> output_off_delta7_f;

    Serializer<unsigned int> output_off_delta8_sr;
    ReadableStateField<unsigned int> output_off_delta8_f;

    Serializer<unsigned int> latchup1_sr;
    ReadableStateField<unsigned int> latchup1_f;

    Serializer<unsigned int> latchup2_sr;
    ReadableStateField<unsigned int> latchup2_f;

    Serializer<unsigned int> latchup3_sr;
    ReadableStateField<unsigned int> latchup3_f;

    Serializer<unsigned int> latchup4_sr;
    ReadableStateField<unsigned int> latchup4_f;

    Serializer<unsigned int> latchup5_sr;
    ReadableStateField<unsigned int> latchup5_f;

    Serializer<unsigned int> latchup6_sr;
    ReadableStateField<unsigned int> latchup6_f;

    Serializer<unsigned int> wdt_i2c_time_left_sr;
    ReadableStateField<unsigned int> wdt_i2c_time_left_f;

    Serializer<unsigned int> wdt_gnd_time_left_sr;
    ReadableStateField<unsigned int> wdt_gnd_time_left_f;

    //chars
    Serializer<unsigned char> wdt_csp_pings_left1_sr;
    ReadableStateField<unsigned char> wdt_csp_pings_left1_f;

    Serializer<unsigned char> wdt_csp_pings_left2_sr;
    ReadableStateField<unsigned char> wdt_csp_pings_left2_f;

    Serializer<unsigned int> counter_wdt_i2c_sr;
    ReadableStateField<unsigned int> counter_wdt_i2c_f;

    Serializer<unsigned int> counter_wdt_gnd_sr;
    ReadableStateField<unsigned int> counter_wdt_gnd_f;

    Serializer<unsigned int> counter_wdt_csp1_sr;
    ReadableStateField<unsigned int> counter_wdt_csp1_f;

    Serializer<unsigned int> counter_wdt_csp2_sr;
    ReadableStateField<unsigned int> counter_wdt_csp2_f;

    Serializer<unsigned int> counter_boot_sr;
    ReadableStateField<unsigned int> counter_boot_f;

    //short ints
    Serializer<short int> temp1_sr;
    ReadableStateField<short int> temp1_f;

    Serializer<short int> temp2_sr;
    ReadableStateField<short int> temp2_f;

    Serializer<short int> temp3_sr;
    ReadableStateField<short int> temp3_f;

    Serializer<short int> temp4_sr;
    ReadableStateField<short int> temp4_f;

    Serializer<short int> temp5_sr;
    ReadableStateField<short int> temp5_f;

    Serializer<short int> temp6_sr;
    ReadableStateField<short int> temp6_f;

    Serializer<unsigned char> bootcause_sr;
    ReadableStateField<unsigned char> bootcause_f;

    Serializer<unsigned char> battmode_sr;
    ReadableStateField<unsigned char> battmode_f;

    Serializer<unsigned char> pptmode_sr;
    ReadableStateField<unsigned char> pptmode_f;

    Serializer<unsigned short int> reserved2_sr;
    ReadableStateField<unsigned short int> reserved2_f;

};

#endif
