#ifndef DESKTOP
#include "TempSensor.hpp"

using namespace Devices;

TempSensor::TempSensor(const std::string &name, unsigned char pin) : Device(name), pin_(pin) {}

bool TempSensor::setup() { return true; }

bool TempSensor::is_functional() { return true; }

void TempSensor::disable() {}
void TempSensor::reset() {}

float TempSensor::get() {
    // TODO implement
    return 0.0f;
}
#endif