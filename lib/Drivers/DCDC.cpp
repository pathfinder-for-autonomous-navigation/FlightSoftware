#include "DCDC.hpp"
#ifndef DESKTOP
#include "Arduino.h"

using namespace Devices;

DCDC::DCDC(const std::string &name, unsigned char en) : Device(name), enable_pin_(en) {}

bool DCDC::setup() {
    pinMode(enable_pin_, OUTPUT);
    digitalWrite(enable_pin_, OFF);
    is_enabled = false;
    return true;
}

bool DCDC::is_functional() { return is_enabled; }

void DCDC::disable() {
    digitalWrite(enable_pin_, OFF);
    is_enabled = false;
}

void DCDC::enable() {
    digitalWrite(enable_pin_, ON);
    is_enabled = true;
}

void DCDC::reset() {
    disable();
    delay(10);
    enable();
}
#endif