
#include "DCDC.hpp"
#ifndef DESKTOP
#include "Arduino.h"
#endif

using namespace Devices;

bool DCDC::setup() {
    #ifndef DESKTOP
    pinMode(dcdc_motor_enable_pin, OUTPUT);
    pinMode(dcdc_sph_enable_pin, OUTPUT);
    digitalWrite(dcdc_motor_enable_pin, LOW);
    digitalWrite(dcdc_sph_enable_pin, LOW);
    #endif
    return true;
}

bool DCDC::is_functional() {
    return true;
}

void DCDC::disable() {
    disable_adcs();
    disable_sph();
    #ifdef DESKTOP
    adcs=false;
    sph=false;
    #endif
}

void DCDC::enable_adcs() {
    #ifndef DESKTOP
    digitalWrite(dcdc_motor_enable_pin, HIGH);
    #else
    adcs=true;
    #endif
}

const bool DCDC::adcs_enabled() {
    #ifndef DESKTOP
    return digitalRead(dcdc_sph_enable_pin);
    #else
    return adcs;
    #endif
}

void DCDC::enable_sph() {
    #ifndef DESKTOP
    digitalWrite(dcdc_sph_enable_pin, HIGH);
    #else
    sph=true;
    #endif
}

const bool DCDC::sph_enabled() {
    #ifndef DESKTOP
    return digitalRead(dcdc_sph_enable_pin);
    #else
    return sph;
    #endif
}

void DCDC::disable_adcs() {
    #ifndef DESKTOP
    digitalWrite(dcdc_motor_enable_pin, LOW);
    #else
    adcs=false;
    #endif
}

void DCDC::disable_sph() {
    #ifndef DESKTOP
    digitalWrite(dcdc_sph_enable_pin, LOW);
    #else
    sph=false; 
    #endif
}

void DCDC::reset() {
    #ifndef DESKTOP
    disable_adcs();
    disable_sph();
    delay(10);
    enable_sph();
    enable_adcs();
    #else
    disable_adcs();
    disable_sph();
    enable_sph();
    enable_adcs();
    #endif
}
