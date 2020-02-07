
#include "DCDC.hpp"
#ifndef DESKTOP
#include "Arduino.h"
#endif

using namespace Devices;

bool DCDC::setup() {
    #ifndef DESKTOP
    pinMode(ADCSMotorDCDC_EN, OUTPUT);
    pinMode(SpikeDockDCDC_EN, OUTPUT);
    digitalWrite(ADCSMotorDCDC_EN, LOW);
    digitalWrite(SpikeDockDCDC_EN, LOW);
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
    digitalWrite(ADCSMotorDCDC_EN, HIGH);
    #else
    adcs=true;
    #endif
}

const bool DCDC::adcs_enabled() {
    #ifndef DESKTOP
    return digitalRead(ADCSMotorDCDC_EN);
    #else
    return adcs;
    #endif
}

void DCDC::enable_sph() {
    #ifndef DESKTOP
    digitalWrite(SpikeDockDCDC_EN, HIGH);
    #else
    sph=true;
    #endif
}

const bool DCDC::sph_enabled() {
    #ifndef DESKTOP
    return digitalRead(SpikeDockDCDC_EN);
    #else
    return sph;
    #endif
}

void DCDC::disable_adcs() {
    #ifndef DESKTOP
    digitalWrite(ADCSMotorDCDC_EN, LOW);
    #else
    adcs=false;
    #endif
}

void DCDC::disable_sph() {
    #ifndef DESKTOP
    digitalWrite(SpikeDockDCDC_EN, LOW);
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
