#include <Arduino.h>

// This code is intended to be run with a 3.6 FC connected
// over I2c to a 3.5 ADCSC with NO devices connected to it to
// verify MOST of HAVT functionality

// Start with a clean boot of the ADCSC with no failures

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

#include <ADCS.hpp>
#include <adcs/constants.hpp>

Devices::ADCS adcs_d(Wire, Devices::ADCS::ADDRESS);

#include <string>
#include <unity.h>

// tests have passed where this is 10, try increment higher if not passing
constexpr static unsigned int wait_for_ADCSC = 10;

// test to make sure you're connected correctly to i2c
void test_get_who_am_i(){
    unsigned char temp;
    adcs_d.get_who_am_i(&temp);
    
    TEST_ASSERT_EQUAL(Devices::ADCS::WHO_AM_I_EXPECTED, temp);
}

// initial test of read capability, will fail if not a clean boot
// ex if there was a previous set_havt command that failed
void test_read_table(){
    std::bitset<adcs::havt::max_devices> read_table_read(0);
    std::bitset<adcs::havt::max_devices> rt_expected("00000000000000000000001110111000");

    Serial.printf("ref havt: %u\n", rt_expected.to_ulong());
    adcs_d.get_havt(&read_table_read);
    Serial.printf("read havt: %u\n", read_table_read.to_ulong());

    //.to_string().c_str() turns a bitset into a string of 0 and 1 like
    //"00000000000000000000001110111000"
    TEST_ASSERT_EQUAL_STRING(rt_expected.to_string().c_str(), read_table_read.to_string().c_str());
}

// disables then re-enables the RWs
void test_cmd_table(){
    // rt stands for read_table
    std::bitset<adcs::havt::max_devices> rt_read(0);
    std::bitset<adcs::havt::max_devices> rt_expected("00000000000000000000001110111000");

    adcs_d.get_havt(&rt_read);

    TEST_ASSERT_EQUAL_STRING(rt_expected.to_string().c_str(), rt_read.to_string().c_str());

    // command table 1 - disable all the reaction wheels, but leave MTR's up
    std::bitset<adcs::havt::max_devices> cmd_t1("00000000000000000000000000111000");

    adcs_d.set_havt(cmd_t1);
    
    // ensure ADCSC has enough time to implement the commanded havt_table
    delay(wait_for_ADCSC);
    adcs_d.get_havt(&rt_read);

    // check that cmd_t1 was applied
    TEST_ASSERT_EQUAL_STRING(cmd_t1.to_string().c_str(), rt_read.to_string().c_str());


    // NOW TEST RESET CAPABILITY
    std::bitset<adcs::havt::max_devices> cmd_t2("00000000000000000000001110111000");
    adcs_d.set_havt(cmd_t2);
    delay(wait_for_ADCSC);
    adcs_d.get_havt(&rt_read);
    TEST_ASSERT_EQUAL_STRING(cmd_t2.to_string().c_str(), rt_read.to_string().c_str());
}

void setup_test() {
    Serial.begin(9600);
    while(!Serial)
        ;
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
    adcs_d.setup();
}

int main(void) {
    delay(1000);
    setup_test();
    UNITY_BEGIN();
    RUN_TEST(test_get_who_am_i);
    RUN_TEST(test_read_table);
    RUN_TEST(test_cmd_table);
    UNITY_END();
    return 0;
}
