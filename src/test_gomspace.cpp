#include <Arduino.h>
#include <Gomspace.hpp>

using namespace Devices;

Gomspace::eps_hk_t hk_data;
Gomspace::eps_config_t config_data;
Gomspace::eps_config2_t config2_data;
Gomspace gs(&hk_data, &config_data, &config2_data);

void print_data(Gomspace::eps_hk_t& hk_data) {
    Serial.print("Vboost: ");
    for(unsigned int i=0; i<3; i++){
        Serial.printf("%hi,", hk_data.vboost[i]);
    }
    Serial.println();
    Serial.printf("VBatt: %hi\n", hk_data.vbatt);
    Serial.printf("Curin: ");
    for(unsigned int i=0; i<3; i++){
        Serial.printf("%hi,", hk_data.curin[i]);
    }
    Serial.printf("\n");
    Serial.printf("Cursun: %hi\n", hk_data.cursun);
    Serial.printf("Cursys: %hi\n", hk_data.cursys);
    Serial.printf("Reserved1: %hi\n", hk_data.reserved1);
    Serial.printf("Curout: ");
    for(unsigned int i=0; i<6; i++){
        Serial.printf("%hi ", hk_data.curout[i]);
    }
    Serial.printf("\n");
    Serial.printf("Output: ");
    for(unsigned int i=0; i<8; i++){
        //data type is unsigned char, so I may need to change the reference code
        Serial.printf("%hi ", hk_data.output[i]);
    }
    Serial.printf("\n");
    Serial.printf("Output on Delta: ");
    for(unsigned int i=0; i<8; i++){
        //data type is unsigned char, so I may need to change the reference code
        Serial.printf("%hi ", hk_data.output_on_delta[i]);
    }
    Serial.printf("\n");
    Serial.printf("Output off Delta: ");
    for(unsigned int i=0; i<8; i++){
        //data type is unsigned char, so I may need to change the reference code
        Serial.printf("%hi ", hk_data.output_off_delta[i]);
    }
    Serial.printf("\n");
    Serial.printf("Latchup: ");
    for(unsigned int i=0; i<6; i++){
        //data type is unsigned char, so I may need to change the reference code
        Serial.printf("%hi ", hk_data.latchup[i]);
    }
    Serial.printf("\n");

    Serial.printf("WDT I2C Time Left: %u\n", hk_data.wdt_i2c_time_left);
    Serial.printf("WDT GND Time Left: %u\n", hk_data.wdt_gnd_time_left);
    Serial.printf("WDT CSP Pings Left: ");
    for(unsigned int i=0; i<2; i++){
        //data type is unsigned char, may need to change reference code
        Serial.printf("%hi ", hk_data.wdt_csp_pings_left[i]);
    }
    Serial.printf("\n");
    Serial.printf("Counter WDT I2C: %u\n", hk_data.counter_wdt_i2c);
    Serial.printf("Counter WDT GND: %u\n", hk_data.counter_wdt_gnd);
    Serial.printf("Counter WDT CSP: ");
    for(unsigned int i=0; i<2; i++){
        Serial.printf("%u ", hk_data.counter_wdt_csp[i]);
    }
    Serial.printf("\n");
    Serial.printf("Counter Boot: %u\n", hk_data.counter_boot);
    Serial.printf("Temp: ");
    for(unsigned int i=0; i<4; i++){
        //data type is a short int. May need to change reference code
        Serial.printf("%i ", hk_data.temp[i]);
    }
    Serial.printf("\n");
    //data type is unsigned char. May need to change reference code
    Serial.printf("Bootcause: %d\n", hk_data.bootcause);
    //data type is unsigned char. May need to change reference code
    Serial.printf("Battery Mode: %d\n", hk_data.battmode);
    delay(10);
    //data type is unsigned char. May need to change reference code hi
    Serial.printf("PPT Mode: %d\n", hk_data.pptmode);
}

void setup() {
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
    Serial.begin(9600);
    while(!Serial);

    gs.setup();
}

void loop() {
    unsigned int data_fetch_time = micros();
    unsigned int num_tries = 1;
    while (!gs.get_hk()) { num_tries++; }
    data_fetch_time = micros() - data_fetch_time;
    print_data(hk_data);
    Serial.printf("Time to fetch data: %d us\n", data_fetch_time);
    Serial.printf("# tries to fetch data: %d\n", num_tries);
    Serial.println("--------------");

    delay(1000);
}
