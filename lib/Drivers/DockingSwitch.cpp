#ifndef DESKTOP
#include "DockingSwitch.hpp"
#include "Arduino.h"

using namespace Devices;

DockingSwitch::DockingSwitch(const std::string &name, unsigned char sw)
    : Device(name), switch_pin_(sw) {}

bool DockingSwitch::setup() {
    pinMode(switch_pin_, INPUT);
    return true;
}

bool DockingSwitch::is_functional() { return true; }

void DockingSwitch::disable() {}
void DockingSwitch::reset() {}

bool DockingSwitch::pressed() { return (digitalRead(switch_pin_) == 1); }
#endif