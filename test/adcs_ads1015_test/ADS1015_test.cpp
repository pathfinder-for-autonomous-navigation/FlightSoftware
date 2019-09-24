//
// test/adcs/ADS1015Test.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#include <ADS1015.hpp>
#include <i2c_t3.h>
#include <Arduino.h>

// ADS Face 6 - P17
ADS1015 adc(&Wire2, ADS1015::ADDR::SSCL, 28, 1000000);

void setup() {
    delay(5000);  // Wait before starting

    pinMode(28, INPUT);
    Serial.begin(9600);
    Wire2.begin(I2C_MASTER, 0x00, I2C_PINS_3_4, I2C_PULLUP_EXT, 400000);
    adc.set_gain(ADS1015::GAIN::ONE);

    if (!adc.init()) {
        Serial.println("Error");
        while (1)
            ;
    } else
        Serial.println("Initialized");
}

// For new version ads1015_2_0
static ADS1015::CHANNEL channels[4] = {ADS1015::CHANNEL::SINGLE_0, ADS1015::CHANNEL::SINGLE_1,
                                       ADS1015::CHANNEL::SINGLE_2, ADS1015::CHANNEL::SINGLE_3};
static int n = 0;

void loop() {
    int16_t v[4];
    bool flag = true;
    // For new version ads1015_2_0
    for (unsigned int i = 0; i < 4; i++) flag &= adc.read(channels[i], v[i]);
    // For old version ads1015_1_0
    // for (unsigned int i = 0; i < 4; i++) flag &= adc.read(i, v[i]);
    if (flag)
        Serial.println(String(n++) + " : " + String(v[0]) + "," + String(v[1]) + "," +
                       String(v[2]) + "," + String(v[3]));
    else
        Serial.println("Error");
    delay(1000);
}
