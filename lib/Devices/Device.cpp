#include "Device.hpp"

using namespace Devices;

Device::Device(const std::string& n) : name_(n) {}

Device::~Device() {}

const std::string& Device::name() const {
    return name_;
}

bool Device::setup() { return true; }
bool Device::is_functional() { return true; }
void Device::reset() { }
void Device::disable() { }