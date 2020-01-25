//
// src/adcs/dev/AD5254.cpp
// FlightSoftware
//
// Contributors:
//   Nathan Zimmerberg  nhz2@cornell.edu
//   Kyle Krol          kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#include "AD5254.hpp"

namespace adcs {
namespace dev {

void AD5254::setup(i2c_t3 *wire, uint8_t addr, unsigned long timeout) {
  this->I2CDevice::setup(wire, addr, timeout);
}

bool AD5254::reset() {
  this->I2CDevice::reset();
  this->set_rdac0(AD5254_RDAC_DEFAULT);
  this->set_rdac1(AD5254_RDAC_DEFAULT);
  this->set_rdac2(AD5254_RDAC_DEFAULT);
  this->set_rdac3(AD5254_RDAC_DEFAULT);
  while (this->is_functional())
    if (this->write_rdac()) return true;
  return false;
}

void AD5254::disable() {
  this->set_rdac0(AD5254_RDAC_DEFAULT);
  this->set_rdac1(AD5254_RDAC_DEFAULT);
  this->set_rdac2(AD5254_RDAC_DEFAULT);
  this->set_rdac3(AD5254_RDAC_DEFAULT);
  this->write_rdac();
  return this->Device::disable();
}

void AD5254::start_write_rdac() {
  this->i2c_begin_transmission();
  this->i2c_write((uint8_t)0x00);
  this->i2c_write(this->rdac, 4);
  this->i2c_send_transmission();
}

bool AD5254::end_write_rdac() {
  this->i2c_finish();
  return !this->i2c_pop_errors();
}

bool AD5254::write_rdac() {
  this->start_write_rdac();
  return this->end_write_rdac();
}
}  // namespace dev
}  // namespace adcs
