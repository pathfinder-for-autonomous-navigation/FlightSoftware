#include "DCDC.hpp"
#include "Arduino.h"

using namespace Devices;

DCDC::DCDC(unsigned char en) : enable_pin_(en) {}

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

void DCDC::single_comp_test() { }

static std::string dcdc_name = "DCDC";
std::string& DCDC::name() const { return dcdc_name; }