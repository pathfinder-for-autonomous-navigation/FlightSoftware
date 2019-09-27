#include <Arduino.h>
#include <i2c_t3.h>
#include <ADCS.hpp>

Devices::ADCS adcs("adcs", Wire, Devices::ADCS::ADDRESS);

#ifndef UNIT_TEST
void setup() {
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
    adcs.setup();
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

    //no way to test this;
    Serial.printf("set_mtr_command: %d\n", test_set_mtr_command());

    //learning bitwise ops
    unsigned short orig = 50;

    unsigned char a = orig >> 8;
    unsigned char b = orig & 0xFF;

    unsigned short c = (((unsigned short)a) << 8) | (0xFF & b);

    //short c = (((short)a) << 8) | (0x00ff & b);

    Serial.printf("test: %u\n",c);

    Serial.println("");
    delay(1000);
}
#endif
