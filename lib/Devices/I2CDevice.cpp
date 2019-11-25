/** @file I2CDevice.cpp
 * @author Tanishq Aggarwal
 * @author Kyle Krol
 * @date 6 Feb 2018
 * @brief Contains implementation for I2CDevice interface, which standardizes
 * the access
 * and control of I2C-based hardware peripherals.
 */
#include "I2CDevice.hpp"

using namespace Devices;

bool I2CDevice::setup() {
    for (uint32_t i = 0; i < I2CDEVICE_DISABLE_AT; i++)
        if (this->i2c_ping())
            return true;
        else
            this->error_count++;
    return false;
}

bool I2CDevice::is_functional() { return (this->error_count < I2CDEVICE_DISABLE_AT); }

void I2CDevice::reset() {
    this->error_count = 0;
    this->recent_errors = false;
}

void I2CDevice::disable() {
    this->error_count = I2CDEVICE_DISABLE_AT;
    this->recent_errors = true;
}

#ifdef DESKTOP
    I2CDevice::I2CDevice(const std::string &name, unsigned long timeout)
        : Device(name), timeout(timeout), error_count(0), recent_errors(false) {}
#else
    I2CDevice::I2CDevice(const std::string &name, i2c_t3 &wire, unsigned char addr,
                        unsigned long timeout)
        : Device(name), wire(wire), addr(addr), timeout(timeout), error_count(0), recent_errors(false) {}
#endif
