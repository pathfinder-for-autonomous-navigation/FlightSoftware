#include "TempSensor.hpp"

using namespace Devices;

TempSensor::TempSensor(unsigned char pin) : pin_(pin) {}

bool TempSensor::setup() {
    return true;
}

bool TempSensor::is_functional() {
    return true;
}

void TempSensor::disable() {}
void TempSensor::reset() {}

static std::string temp_sensor_name = "Temperature Sensor";
std::string& TempSensor::name() const {
    return temp_sensor_name;
}

float TempSensor::get() {
    // TODO implement
    return 0.0f;
}