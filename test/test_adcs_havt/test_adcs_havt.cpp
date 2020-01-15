#include "Arduino.h"

// This code is intended to be run with a 3.6 FC connected
// over I2c to a 3.5 ADCSC with NO devices connected to it to
// verify MOST of HAVT functionality

// Define the below block or add these build flags
// -llibc -lc
// in order to overcome the undefined reference to _write()

// extern "C"{
//   int _getpid(){ return -1;}
//   int _kill(int pid, int sig){ return -1; }
//   int _write(){return -1;}
// }

// removes mocking from Driver
// -UUNIT_TEST

#include "../lib/Drivers/ADCS.hpp"
#include <adcs_constants.hpp>

Devices::ADCS adcs(Wire, Devices::ADCS::ADDRESS);

#include <string>
#include <unity.h>

void test_trivial(){
    TEST_ASSERT_TRUE(true);
}

void test_get_who_am_i(){
    unsigned char temp;
    adcs.get_who_am_i(&temp);
    
    TEST_ASSERT_EQUAL(Devices::ADCS::WHO_AM_I_EXPECTED, temp);
}

void test_read_table(){
    std::bitset<havt::max_devices> read_table_read(0);
    std::bitset<havt::max_devices> rt_expected("00000000000000000000001110111000");

    Serial.printf("ref havt: %u\n", rt_expected.to_ulong());
    adcs.get_havt(&read_table_read);
    Serial.printf("read havt: %u\n", read_table_read.to_ulong());

    TEST_ASSERT_EQUAL_STRING(rt_expected.to_string().c_str(), read_table_read.to_string().c_str());
}

void test_setup() {
    Serial.begin(9600);
    while(!Serial)
        ;
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
    adcs.setup();
}

int main(void) {
    delay(1000);
    test_setup();
    UNITY_BEGIN();
    RUN_TEST(test_trivial);
    RUN_TEST(test_get_who_am_i);
    RUN_TEST(test_read_table);
    UNITY_END();
    return 0;
}
