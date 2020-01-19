//
// test/motorTest.cpp
// ADCS
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

/* Runs motor circuit two with a servo pulse speed control.
 * Currently hangs for unknown reason - DO NOT USE! 
 * Use test/rwaTest.cpp instead. */

#define DEBUG
#include <debug.hpp>

#include <Arduino.h>
#include <i2c_t3.h>
#include <Servo.h>

#include <AD5254.hpp>
#include <ADS1015.hpp>
#include <state_constants.hpp>

#define RWA_WHEEL0_CW_PIN 26
#define RWA_WHEEL0_CCW_PIN 39
#define RWA_WHEEL0_SPEED_PIN 23

Servo myservo;
AD5254 potentiometer;
ADS1015 analogToDigital;

void setup() {

    delay(5000);
    DEBUG_init(9600)
    DEBUG_printlnF("Initializing i2c bus")

    analogWriteResolution(16);

    pinMode(rwa::wheel1_cw_pin, OUTPUT);
    pinMode(rwa::wheel1_ccw_pin, OUTPUT);
    pinMode(rwa::wheel1_speed_pin, OUTPUT);

    Wire1.begin(I2C_MASTER, 0x00, I2C_PINS_37_38, I2C_PULLUP_EXT, 400000);
    potentiometer.setup(rwa::pot_wire, rwa::pot_addr, rwa::pot_timeout);
    potentiometer.reset();
    potentiometer.set_rdac0(255);
    potentiometer.set_rdac1(255);
    potentiometer.set_rdac2(255);
    potentiometer.set_rdac3(255);
    potentiometer.write_rdac();

    analogToDigital.setup(rwa::adc1_wire, rwa::adc1_addr, rwa::adc1_alrt, rwa::adcx_timeout);
    analogToDigital.set_gain(ADS1015::GAIN::ONE);
    analogToDigital.reset();

    digitalWrite(rwa::wheel1_cw_pin, LOW);
    digitalWrite(rwa::wheel1_ccw_pin, LOW);

    myservo.attach(rwa::wheel1_speed_pin);
    myservo.writeMicroseconds(2000);

    for (unsigned int i = 0; i < 5; i++) {

        DEBUG_printlnF("Looping...")

        digitalWrite(rwa::wheel1_cw_pin, HIGH);
        digitalWrite(rwa::wheel1_ccw_pin, LOW);

        for (unsigned int j = 0; j < 10; j++) {
            int16_t val;

            DEBUG_printlnF("Here")

            analogToDigital.read(ADS1015::CHANNEL::DIFFERENTIAL_0_1, val);
            
            DEBUG_printlnF("Here again")
            
            DEBUG_print(String(val) + ",")
            delay(200);
        }

        digitalWrite(rwa::wheel1_cw_pin, LOW);
        digitalWrite(rwa::wheel1_ccw_pin, HIGH);

        for (unsigned int j = 0; j < 10; j++) {
            int16_t val;
            analogToDigital.read(ADS1015::CHANNEL::DIFFERENTIAL_0_1, val);
            DEBUG_print(String(val) + ",")
            delay(400);
        }
        DEBUG_println()

        digitalWrite(rwa::wheel1_cw_pin, HIGH);
        digitalWrite(rwa::wheel1_ccw_pin, LOW);

        for (unsigned int j = 0; j < 10; j++) {
            int16_t val;
            analogToDigital.read(ADS1015::CHANNEL::DIFFERENTIAL_0_1, val);
            DEBUG_print(String(val) + ",")
            delay(200);
        }
        DEBUG_println()

        digitalWrite(rwa::wheel1_cw_pin, LOW);
        digitalWrite(rwa::wheel1_ccw_pin, LOW);

        for (unsigned int j = 0; j < 10; j++) {
            int16_t val;
            analogToDigital.read(ADS1015::CHANNEL::DIFFERENTIAL_0_1, val);
            DEBUG_print(String(val) + ",")
            delay(100);
        }
        DEBUG_println()

    }

    DEBUG_printlnF("Complete...")

}

void loop() {
    // empty
}
