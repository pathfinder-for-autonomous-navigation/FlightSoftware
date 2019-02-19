#include "PressureSensor.hpp"

using namespace Devices;

PressureSensor::PressureSensor(unsigned char l, unsigned char h) : low_pressure_pin_(l), high_pressure_pin_(h) {}

bool PressureSensor::setup() {
    return true;
}

bool PressureSensor::is_functional() {
    return true;
}

void PressureSensor::disable() {}
void PressureSensor::reset() {}
void PressureSensor::single_comp_test() {}

static std::string temp_sensor_name = "Pressure Sensor";
std::string& PressureSensor::name() const {
    return temp_sensor_name;
}

float PressureSensor::get() {
    // TODO implement
    return 0.0f;
}