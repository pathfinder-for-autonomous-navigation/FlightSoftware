#include "DCDC.hpp"
#include "Arduino.h"

using namespace Devices;

DCDC::DCDC() : Device("dcdc") {}

bool DCDC::setup() {
    pinMode(dcdc_motor_enable_pin, OUTPUT);
    pinMode(dcdc_sph_enable_pin, OUTPUT);
    digitalWrite(dcdc_motor_enable_pin, OFF);
    digitalWrite(dcdc_sph_enable_pin, OFF);
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
    digitalWrite(dcdc_motor_enable_pin, ON);
}

void DCDC::enable_sph() {
    digitalWrite(dcdc_sph_enable_pin, ON);
}

void DCDC::disable_adcs() {
    digitalWrite(dcdc_motor_enable_pin, OFF);
}

void DCDC::disable_sph() {
    digitalWrite(dcdc_sph_enable_pin, OFF);
}

void DCDC::reset() {
    disable_adcs();
    disable_sph();
    delay(10);
    enable_sph();
    enable_adcs();
}
