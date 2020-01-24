//
// lib/LSM6DSM/LSM6DSM.cpp
// ADCS
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#include "LSM6DSM.hpp"

namespace adcs {
namespace dev {

void LSM6DSM::setup(i2c_t3 *wire, uint8_t addr, unsigned long timeout) {
  this->I2CDevice::setup(wire, addr, timeout);
}

bool LSM6DSM::reset() {
  this->I2CDevice::reset();
  while (this->is_functional()) {
    this->i2c_begin_transmission();
    this->i2c_write(REG::CTRL2_G);
    this->i2c_write(0x32); // 125 dps, 52 Hz, High Performance
    this->i2c_end_transmission();
    if (!this->i2c_pop_errors()) return true;
  }
  this->disable();
  return false;
}

void LSM6DSM::disable() {
  this->I2CDevice::disable();
  this->i2c_begin_transmission();
  this->i2c_write(REG::CTRL2_G);
  this->i2c_write(0x00); // 250 dps, Power down
  this->i2c_end_transmission();
}

bool LSM6DSM::is_ready() {
  this->i2c_begin_transmission(); 
  this->i2c_write(LSM6DSM::REG::STATUS_REG);
  this->i2c_end_transmission();
  this->i2c_request_from(1);
  uint8_t val = this->i2c_read();
  return (!this->i2c_pop_errors() && ((val & 2) != 0));
}

bool LSM6DSM::read() {
  // Read in data if ready
  uint8_t buffer[8];
  this->i2c_begin_transmission();
  this->i2c_write(LSM6DSM::REG::OUT_TEMP_L);
  this->i2c_end_transmission(I2C_NOSTOP);
  this->i2c_request_from(8);
  this->i2c_read(buffer, 8);
  // Check for errors
  if (i2c_pop_errors()) return false;
  // Process data
  this->temp = buffer[0] | (buffer[1] << 8);
  this->omega[0] = buffer[2] | (buffer[3] << 8);
  this->omega[1] = buffer[4] | (buffer[5] << 8);
  this->omega[2] = buffer[6] | (buffer[7] << 8);
  return true;  
}
}  // namespace dev
}  // namespace adcs
