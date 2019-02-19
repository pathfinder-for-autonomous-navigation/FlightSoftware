/** @file I2CDevice.cpp
 * @author Tanishq Aggarwal
 * @author Kyle Krol
 * @date 6 Feb 2018
 * @brief Contains implementation for I2CDevice interface, which standardizes the access
 * and control of I2C-based hardware peripherals.
 */

#include "I2CDevice.hpp"

using namespace Devices;

bool I2CDevice::setup() {
  for (uint32_t i = 0; i < I2CDEVICE_DISABLE_AT; i++)
    if (this->i2c_ping()) return true;
    else this->error_count++;
  return false;
}

bool I2CDevice::is_functional() {
  return (this->error_count < I2CDEVICE_DISABLE_AT);
}

void I2CDevice::reset() {
  this->error_count = 0;
  this->recent_errors = false;
}

void I2CDevice::disable() {
  this->error_count = I2CDEVICE_DISABLE_AT;
  this->recent_errors = true;
}

void I2CDevice::single_comp_test() {
  // TODO
}

static std::string i2cdevice_name = "Generic I2C Device";
std::string& I2CDevice::name() const {
  return i2cdevice_name;
}

I2CDevice::I2CDevice(i2c_t3 &wire, unsigned char addr, unsigned long timeout)
    : wire(wire),
      addr(addr),
      timeout(timeout),
      error_count(0),
      recent_errors(false) {
  // empty
}
