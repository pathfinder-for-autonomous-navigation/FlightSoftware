//
// lib/MMC34160PJ/MMC34160PJ.cpp
// ADCS
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#include "MMC34160PJ.hpp"

void MMC34160PJ::setup(i2c_t3 *wire, unsigned long timeout) {
  this->dev::I2CDevice::setup(wire, 0x30, timeout);
  this->sample_rate = MMC34160PJ::SR::HZ_50;
  this->offset[0] = 0x00;
  this->offset[1] = 0x00;
  this->offset[2] = 0x00;
}

bool MMC34160PJ::reset() {
  this->dev::I2CDevice::reset();
  while (this->is_functional()) {
    this->i2c_begin_transmission();
    this->i2c_write(MMC34160PJ::REG::INTERNAL_CONTROL_0);
    this->i2c_write(this->sample_rate | 0b10); // Sets the SR and cont. mode
    this->i2c_end_transmission(I2C_NOSTOP);
    if (!this->i2c_pop_errors()) return true;
  }
  return false;
}

void MMC34160PJ::disable() {
  this->i2c_begin_transmission();
  this->i2c_write(MMC34160PJ::REG::INTERNAL_CONTROL_0);
  this->i2c_write(0x00); // Disable continous measurement mode
  this->i2c_end_transmission();
  this->dev::I2CDevice::disable();
}

void MMC34160PJ::calibrate() {
  uint16_t set_read[3];
  uint16_t reset_read[3];
  // Perform set operation and read
  this->fill_capacitor();
  this->set_operation();
  delay(1);
  if (this->single_read(set_read)) {
    // Perform the reset operation and read
    this->fill_capacitor();
    this->reset_operation();
    delay(1);
    if (this->single_read(reset_read)) {
      for (unsigned int i = 0; i < 3; i++)
        this->offset[0] = (uint16_t) (
            ( ((unsigned int) set_read[i]) - ((unsigned int) reset_read[i]) ) / 2
                );
    }
  }
  this->I2CDevice::disable();
}

bool MMC34160PJ::is_ready() {
  // Check measurement done bit
  this->i2c_begin_transmission();
  this->i2c_write(MMC34160PJ::REG::STATUS);
  this->i2c_end_transmission(I2C_NOSTOP);
  this->i2c_request_from(1);
  return (!this->i2c_pop_errors()) && (this->i2c_read() & 0b1);
}

bool MMC34160PJ::read() {
  // Set read register
  this->i2c_begin_transmission();
  this->i2c_write(MMC34160PJ::REG::OUT);
  this->i2c_end_transmission(I2C_NOSTOP);
  // Read in measurement data
  uint8_t buffer[6];
  this->i2c_request_from(6);
  this->i2c_read(buffer, 6);
  // Check for errors
  if (this->i2c_pop_errors()) return false;
  // Process data
  this->b_vec[0] = (buffer[0] << 0) | (buffer[1] << 8);
  this->b_vec[1] = (buffer[2] << 0) | (buffer[3] << 8);
  this->b_vec[2] = (buffer[4] << 0) | (buffer[5] << 8);
  return true;
}

void MMC34160PJ::fill_capacitor() {
// Request capacitor refill and wait for it to complete
  this->i2c_begin_transmission();
  this->i2c_write(MMC34160PJ::REG::INTERNAL_CONTROL_0);
  this->i2c_write(0x80);
  this->i2c_end_transmission();
  delay(50);
}

void MMC34160PJ::set_operation() {
  this->i2c_begin_transmission();
  this->i2c_write(MMC34160PJ::REG::INTERNAL_CONTROL_0);
  this->i2c_write(0x20);
  this->i2c_end_transmission();
}

void MMC34160PJ::reset_operation() {
  this->i2c_begin_transmission();
  this->i2c_write(MMC34160PJ::REG::INTERNAL_CONTROL_0);
  this->i2c_write(0x40);
  this->i2c_end_transmission();
}

bool MMC34160PJ::single_read(uint16_t *array) {
  this->i2c_begin_transmission();
  this->i2c_write(MMC34160PJ::REG::INTERNAL_CONTROL_0);
  this->i2c_write(0x01);
  this->i2c_end_transmission();
  while (!this->is_ready())
    if (!this->is_functional()) return false;
  if (!this->read()) return false;
  for (unsigned int i = 0; i < 3; i++) array[i] = this->b_vec[i];
  return true;
}
