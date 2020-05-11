#include <core_pins.h>
#include <usb_serial.h>
#include <fsw/FCCode/Drivers/Gomspace.hpp>
#include <unity.h>

using namespace Devices;

Gomspace::eps_hk_t hk_data;
Gomspace::eps_config_t config_data;
Gomspace::eps_config2_t config2_data;
Gomspace gs(&hk_data, &config_data, &config2_data);

void test_fn() {
    TEST_ASSERT(gs.is_functional());
    
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
}

void test_vbatt() {
    if (gs.get_hk()){
        Serial.printf("VBatt: %hi\n", hk_data.vbatt);
        TEST_ASSERT_GREATER_THAN(7000, hk_data.vbatt);
    }
}

void test_curin() {
    if (gs.get_hk()){
        Serial.printf("Curin: ");
        for(unsigned int i=0; i<2; i++){
            Serial.printf("%hi ", hk_data.curin[i]);
        }
        Serial.printf("\n");
    }
}

void test_cursun() {
    if (gs.get_hk()){
        Serial.printf("Cursun: %hi\n", hk_data.cursun);
    }
}

void test_cursys() {
    if (gs.get_hk()){
        Serial.printf("Cursys: %hi\n", hk_data.cursys);
    }
}

void test_reserved1() {
    if (gs.get_hk()){
        Serial.printf("Reserved1: %hi\n", hk_data.reserved1);
    }
}

void test_curout() {
    if (gs.get_hk()){
        Serial.printf("Curout: ");
        for(unsigned int i=0; i<6; i++){
            Serial.printf("%hi ", hk_data.curout[i]);
        }
        Serial.printf("\n");
    }
}

void test_output() {
    if (gs.get_hk()){
        Serial.printf("Output: ");
        for(unsigned int i=0; i<8; i++){
            //data type is unsigned char, so I may need to change the reference code
            Serial.printf("%hi ", hk_data.output[i]);
        }
        Serial.printf("\n");
    }
}

void test_output_on_delta() {
    if (gs.get_hk()){
        Serial.printf("Output on Delta: ");
        for(unsigned int i=0; i<8; i++){
            //data type is unsigned char, so I may need to change the reference code
            Serial.printf("%hi ", hk_data.output_on_delta[i]);
        }
        Serial.printf("\n");
    }
}

void test_output_off_delta() {
    if (gs.get_hk()){
        Serial.printf("Output off Delta: ");
        for(unsigned int i=0; i<8; i++){
            //data type is unsigned char, so I may need to change the reference code
            Serial.printf("%hi ", hk_data.output_off_delta[i]);
        }
        Serial.printf("\n");
    }
}

void test_latchup() {
    if (gs.get_hk()){
        Serial.printf("Latchup: ");
        for(unsigned int i=0; i<6; i++){
            //data type is unsigned char, so I may need to change the reference code
            Serial.printf("%hi ", hk_data.latchup[i]);
        }
        Serial.printf("\n");
    }
}

void test_wdt_i2c_time_left(){
    if (gs.get_hk()){
        Serial.printf("WDT I2C Time Left: %u\n", hk_data.wdt_i2c_time_left);
    }
}

void test_wdt_gnd_time_left(){
    if (gs.get_hk()){
        Serial.printf("WDT GND Time Left: %u\n", hk_data.wdt_gnd_time_left);
    }
}

void test_wdt_csp_pings_left() {
    if (gs.get_hk()){
        Serial.printf("WDT CSP Pings Left: ");
        for(unsigned int i=0; i<2; i++){
            //data type is unsigned char, may need to change reference code
            Serial.printf("%hi ", hk_data.wdt_csp_pings_left[i]);
        }
        Serial.printf("\n");
    }
}

void test_counter_wdt_i2c(){
    if (gs.get_hk()){
        Serial.printf("Counter WDT I2C: %u\n", hk_data.counter_wdt_i2c);
    }
}

void test_counter_wdt_gnd(){
    if (gs.get_hk()){
        Serial.printf("Counter WDT GND: %u\n", hk_data.counter_wdt_gnd);
    }
}

void test_counter_wdt_csp() {
    if (gs.get_hk()){
        Serial.printf("Counter WDT CSP: ");
        for(unsigned int i=0; i<2; i++){
            Serial.printf("%u ", hk_data.counter_wdt_csp[i]);
        }
        Serial.printf("\n");
    }
}

void test_counter_boot(){
    if (gs.get_hk()){
        Serial.printf("Counter Boot: %u\n", hk_data.counter_boot);
    }
}

void test_temp() {
    if (gs.get_hk()){
        Serial.printf("Temp: ");
        for(unsigned int i=0; i<4; i++){
            //data type is a short int. May need to change reference code
            Serial.printf("%i ", hk_data.temp[i]);
        }
        Serial.printf("\n");
    }
}

void test_bootcause() {
    if (gs.get_hk()){
        //data type is unsigned char. May need to change reference code
        Serial.printf("Bootcause: %f\n", hk_data.bootcause);
    }
}

void test_battmode() {
    if (gs.get_hk()){
        //data type is unsigned char. May need to change reference code
        Serial.printf("Battery Mode: %f", hk_data.battmode);
        delay(10);
    }
}

void test_pptmode() {
    if (gs.get_hk()){
        //data type is unsigned char. May need to change reference code
        Serial.printf("PPT Mode: %f", hk_data.pptmode);
    }
}

void test_vboost(){
    if (gs.get_hk()){
        Serial.println(sizeof(hk_data.vboost));
        for(unsigned int i=0; i<3; i++){
            Serial.printf("%hi\n", hk_data.vboost[i]);
        }
    }
}

int main() {
    delay(6000);
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
    Serial.begin(9600);
    while(!Serial);
    gs.setup();
    UNITY_BEGIN();
    RUN_TEST(test_fn);
    RUN_TEST(test_vbatt);
    RUN_TEST(test_curin);
    RUN_TEST(test_cursun);
    RUN_TEST(test_cursys);
    RUN_TEST(test_reserved1);
    RUN_TEST(test_curout);
    RUN_TEST(test_output);
    RUN_TEST(test_output_on_delta);
    RUN_TEST(test_output_off_delta);
    RUN_TEST(test_latchup);
    RUN_TEST(test_wdt_i2c_time_left);
    RUN_TEST(test_wdt_gnd_time_left);
    RUN_TEST(test_wdt_csp_pings_left);
    RUN_TEST(test_counter_wdt_i2c);
    RUN_TEST(test_counter_wdt_gnd);
    RUN_TEST(test_counter_wdt_csp);
    RUN_TEST(test_counter_boot);
    RUN_TEST(test_temp);
    RUN_TEST(test_bootcause);
    RUN_TEST(test_battmode);
    RUN_TEST(test_pptmode);
    RUN_TEST(test_vboost);
    UNITY_END();
    return 0;
}
