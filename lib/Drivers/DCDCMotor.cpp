#include "DCDC.hpp"
#include "Arduino.h"

using namespace Devices;

DCDCMotor::DCDCMotor() : Device("dcdc_motor") {}

bool DCDCMotor::setup() {
    pinMode(dcdc_motor_enable_pin, OUTPUT);
    digitalWrite(dcdc_motor_enable_pin, OFF);
    is_enabled = false;
    return true;
}

bool DCDCMotor::is_functional() { return is_enabled; }

void DCDCMotor::disable() {
    digitalWrite(dcdc_motor_enable_pin, OFF);
    is_enabled = false;
}

void DCDCMotor::enable() {
    digitalWrite(dcdc_motor_enable_pin, ON);
    is_enabled = true;
}

void DCDCMotor::reset() {
    disable();
    delay(10);
    enable();
}
