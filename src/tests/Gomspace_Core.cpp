#include <Arduino.h>
#include <i2c_t3.h>
#include <Gomspace/Gomspace.hpp>
using namespace Devices;

#define HK_READ_TESTING

Gomspace gs(Wire, Gomspace::ADDRESS);
bool setup_result;

void setup() {
    Serial.begin(9600);
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_INT, 400000, I2C_OP_MODE_ISR);
    delay(100);
    setup_result = gs.setup();
}

void loop() {
    if (gs.get_hk()) {
        Gomspace::eps_hk_t* hk = gs.hk;

        Serial.printf("boost converter voltages (mV): %d,%d,%d\n", hk->vboost[0], hk->vboost[1], hk->vboost[2]);
        Serial.printf("vbattery (mV): %d\n", hk->vbatt);
        Serial.printf("current in (mA): %d,%d,%d\n", hk->curin[0], hk->curin[1], hk->curin[2]);
        Serial.printf("current from boost converters (mA): %d\n", hk->cursun);
        Serial.printf("current out from battery (mA): %d\n", hk->cursys);
        Serial.printf("current out from outputs: %d,%d,%d,%d,%d,%d\n", hk->curout[0], hk->curout[1], hk->curout[2],
            hk->curout[3], hk->curout[4], hk->curout[5]);
        Serial.printf("are outputs on?: %d,%d,%d,%d,%d,%d,%d,%d\n", hk->output[0], hk->output[1], hk->output[2],
            hk->output[3], hk->output[4], hk->output[5], hk->output[6], hk->output[7]);
        Serial.printf("time left on i2c wdt: %d\n", hk->wdt_i2c_time_left);
        Serial.printf("number of reboots due to i2c: %d\n", hk->counter_wdt_i2c);
        Serial.printf("number of reboots of EPS: %d\n", hk->counter_boot);
        Serial.printf("temperature readings: %d,%d,%d,%d,%d,%d\n", hk->temp[0], hk->temp[1],hk->temp[2], hk->temp[3],
            hk->temp[4], hk->temp[5]);
        Serial.printf("cause of last reboot: %d\n", hk->bootcause);
        Serial.printf("current battery mode: %d\n", hk->battmode);
        Serial.printf("current ppt mode: %d\n", hk->pptmode);
    }
    else {
        Serial.println("Failed to fetch housekeeping data.");
    }
    delay(1000);
}