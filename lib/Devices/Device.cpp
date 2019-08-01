#include "Device.hpp"

using namespace Devices;

Device::~Device() {}

bool Device::setup() { return true; }
bool Device::is_functional() { return true; }
void Device::reset() {}
void Device::disable() {}