#include "DCDC.hpp"
#ifndef DESKTOP
#include "Arduino.h"
#endif

using namespace Devices;

DCDC::DCDC() : Device("dcdc") {}

bool DCDC::setup() {
    pinMode(dcdc_motor_enable_pin, OUTPUT);
    pinMode(dcdc_sph_enable_pin, OUTPUT);
    digitalWrite(dcdc_motor_enable_pin, LOW);
    digitalWrite(dcdc_sph_enable_pin, LOW);
    return true;
}

bool DCDC::is_functional() {
    return true;
}

void DCDC::disable() {
    disable_adcs();
    disable_sph();
}

void DCDC::enable_adcs() {
    digitalWrite(dcdc_motor_enable_pin, HIGH);
}

void DCDC::enable_sph() {
    digitalWrite(dcdc_sph_enable_pin, HIGH);
}

void DCDC::disable_adcs() {
    digitalWrite(dcdc_motor_enable_pin, LOW);
}

void DCDC::disable_sph() {
    digitalWrite(dcdc_sph_enable_pin, LOW);
}

void DCDC::reset() {
    disable_adcs();
    disable_sph();
    delay(10);
    enable_sph();
    enable_adcs();
}