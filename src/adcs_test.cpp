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

bool test_get_ssa_mode(){
    unsigned char temp1 = 3;
    adcs.get_ssa_mode(&temp1);
    
    Serial.println(temp1);
    return temp1 == 0;
}

void test_get_ssa_voltage(){
    float temp[20];
    adcs.get_ssa_voltage(temp);

    for(unsigned int i = 0;  i < sizeof(temp)/sizeof(temp[0]); i++){
        Serial.printf("%lf\n",temp[i]);
    }

    unsigned char rawdata[20];
    adcs.get_ssa_voltage_char(rawdata);
    for(unsigned int i = 0;  i < sizeof(temp)/sizeof(temp[0]); i++){
        Serial.printf("%u\n",rawdata[i]);
    }


    // unsigned char temp[20];
    // adcs.get_ssa_voltage_char(temp);
    
    // for(unsigned int i = 0;  i < sizeof(temp)/sizeof(temp[0]); i++){
    //     Serial.printf("%c\n",temp[i]);
    // }

}

void loop() {
    //Serial.println(adcs.is_functional());
    Serial.printf("get_who_am_i: %d\n", test_get_who_am_i());
    delay(100);
    
    Serial.printf("get_ssa_mode: %d\n", test_get_ssa_mode());
    delay(100);

    test_get_ssa_voltage();
    delay(100);

    Serial.printf("3 serialized into a char: %u",(unsigned char)231.81f);
    
    Serial.println("");
    delay(1000);
}
#endif
