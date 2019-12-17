#pragma once

#ifndef GOMSPACE_CONTROLLER_HPP_
#define GOMSPACE_CONTROLLER_HPP_

#include "TimedControlTask.hpp"
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

};

#endif
