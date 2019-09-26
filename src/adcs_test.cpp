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
void loop() {
    //Serial.println(adcs.is_functional());
    Serial.printf("get_who_am_i: %d\n", test_get_who_am_i());
    delay(100);
    
    Serial.printf("get_ssa_mode: %d\n", test_get_ssa_mode());
    delay(100);
    
    Serial.println("");
    delay(1000);
}
#endif
