#include "PressureSensor.hpp"

using namespace Devices;

PressureSensor::PressureSensor(const std::string& name, 
                               unsigned char l, 
                               unsigned char h) : Device(name),
                                                  low_pressure_pin_(l), 
                                                  high_pressure_pin_(h) {}

bool PressureSensor::setup() {
    return true;
}

bool PressureSensor::is_functional() {
    return true;
}

void PressureSensor::disable() {}
void PressureSensor::reset() {}

float PressureSensor::get() {
    // TODO implement
    return 0.0f;
}