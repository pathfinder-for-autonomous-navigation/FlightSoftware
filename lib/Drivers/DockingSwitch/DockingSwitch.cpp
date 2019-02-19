#include "DockingSwitch.hpp"
#include "Arduino.h"

using namespace Devices;

DockingSwitch::DockingSwitch(unsigned char sw) : switch_pin_(sw) {}

bool DockingSwitch::setup() { 
    pinMode(switch_pin_, INPUT);
    return true;
}

bool DockingSwitch::is_functional() { return true; }

void DockingSwitch::disable() { }
void DockingSwitch::reset() { }
void DockingSwitch::single_comp_test() { }

bool DockingSwitch::pressed() {
    return (digitalRead(switch_pin_) == 1);
}

static std::string dockingswitch_name = "Docking Switch";
std::string& DockingSwitch::name() const { return dockingswitch_name; }