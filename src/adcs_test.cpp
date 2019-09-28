#include <Arduino.h>
#include <i2c_t3.h>
#include <ADCS.hpp>
#include <array>

Devices::ADCS adcs("adcs", Wire, Devices::ADCS::ADDRESS);

#ifndef UNIT_TEST
void setup() {
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
    adcs.setup();
}
template <class T, std::size_t N>
bool comp_float_arr(std::array<T,N> a,std::array<T,N> b,float margin){
    bool ret = true;
    for(unsigned int i = 0;i<N;i++){
        //Serial.printf("diff: %f\n",abs(a[i]-b[i]));
        if(abs(a[i]-b[i])>margin)
            ret = false;
    }
    return ret;
}

bool test_get_who_am_i(){
    unsigned char temp = 2;
    adcs.get_who_am_i(&temp);
    
    Serial.println(temp);
    return 15 == temp;
    //Serial.println(temp[0]);
}

bool test_getset_ssa_mode(){
    //state.cpp default is 0
    adcs.set_ssa_mode(1);
    //should change from 4 to 1
    unsigned char temp = 4;
    adcs.get_ssa_mode(&temp);
    return temp == 1;

}
bool test_set_mtr_command(){
    std::array<float,3> cmd = {0.01f,0.01f,-0.01f};

    adcs.set_mtr_cmd(cmd);

    return true;
}
//this is now working
//just not too sure about when rwa mode = 2
//values too small to verify
bool test_set_rwa_mode(){
    std::array<float,3> cmd = {200.0f,400.0f,-500.0f};
    adcs.set_rwa_mode(1,cmd);

    std::array<float,3> cmdAVG = {0.0001f,0.0002f,-0.0003f};
    adcs.set_rwa_mode(2,cmdAVG);

    return true;
}
//should not be needed again
bool test_desperate(){
    unsigned char volt[20];
    adcs.get_ssa_voltage_char(volt);
    for(unsigned int i = 0; i<20;i++){
        Serial.printf("arr: %u\n", volt[i]);
    }
    

    unsigned char rwadesp[12];
    adcs.get_rwa_char(rwadesp);
    for(unsigned int i = 0; i<12;i++){
        Serial.printf("rwadesp: %u\n", rwadesp[i]);
    }
    return false;
}
bool test_get_rwa(){
    //dummy inital values
    std::array<float, 3> rwa_momentum_rd = {1.0f,1.0f,1.0f};
    std::array<float, 3> rwa_ramp_rd = {1.0f,1.0f,1.0f};;

    std::array<float, 3> rwa_momentum_state = {0.004f, 0.005f, -0.006f}; // Momentum read
    std::array<float, 3> rwa_ramp_state = {0.001f, 0.002f, -0.003f};

    adcs.get_rwa(rwa_momentum_rd, rwa_ramp_rd);

    // Serial.printf("float: %f\n",rwa_momentum_rd[0]);
    // Serial.printf("float: %f\n",rwa_momentum_rd[1]);
    // Serial.printf("float: %f\n",rwa_momentum_rd[2]);

    // Serial.printf("float: %f\n",rwa_ramp_rd[0]);
    // Serial.printf("float: %f\n",rwa_ramp_rd[1]);
    // Serial.printf("float: %f\n",rwa_ramp_rd[2]);

    return comp_float_arr(rwa_momentum_rd,rwa_momentum_state,0.0001f);
    //return true;
    //return operator==(rwa_momentum_rd,rwa_momentum_state) && operator==(rwa_ramp_rd,rwa_ramp_state);
    return rwa_momentum_rd == rwa_momentum_state;// && rwa_ramp_rd == rwa_ramp_state;
}
void rwa_rd12(){
    unsigned char test[12] = {1,1,1,1,1,1,1,1,1,1,1,1};
    adcs.get_rwa_char(test);
    for(int i = 0;i<12;i++){
        Serial.printf("char: %u\n",test[i]);
    }

}
void helper(std::array<float, 3>& in){
    in[0] = 0.5f;
}
void test_array_mechanics(){
    std::array<float,3> given = {0.0f,1.0f,2.0f};
    helper(given);

    Serial.printf("helper: %f\n",given[0]);
}
bool test_get_ssa_voltage(){
    float temp[20];
    adcs.get_ssa_voltage(temp);

    for(unsigned int i = 0;  i < sizeof(temp)/sizeof(temp[0]); i++){
        //Serial.printf("%lf\n",temp[i]);
    }

    float reference[20] = 
    {1.0f, 2.0f, 3.0f, 0.0f, 0.0f, // Voltage read
     1.0f, 2.0f, 3.0f, 0.0f, 0.0f,
     1.0f, 2.0f, 3.0f, 0.0f, 0.0f,
     1.0f, 2.0f, 3.0f, 0.0f, 0.0f};

    bool ret = true;
    //check to make sure returned values are within 0.02
    for(unsigned int i=0; i< sizeof(temp)/sizeof(temp[0]); i++){
        if(abs(temp[i]-reference[i])>0.02){
            ret = false;
        }
    }

    return ret;


}

void loop() {
    //Serial.println(adcs.is_functional());
    Serial.printf("get_who_am_i: %d\n", test_get_who_am_i());
    
    Serial.printf("set_ssa_mode: %d\n", test_getset_ssa_mode());

    //Serial.printf("get_ssa_mode: %d\n", test_get_ssa_mode());

    Serial.printf("get_ssa_voltage: %d\n", test_get_ssa_voltage());

    //no way to test this;
    Serial.printf("set_rwa_mode: %d\n", test_set_rwa_mode());

    Serial.printf("get_rwa: %d\n", test_get_rwa());
    //test_array_mechanics();
    //test_desperate();

    //rwa_rd12();

    //no way to test this;
    Serial.printf("set_mtr_command: %d\n", test_set_mtr_command());

    //unsigned short c = (((unsigned short)a) << 8) | (0xFF & b);

    Serial.println("");
    delay(1000);
}
#endif
