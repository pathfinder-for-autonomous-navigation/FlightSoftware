//
// lib/ADS1015/ADS1015.cpp
// ADCS
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#include "ADS1015.hpp"

namespace adcs {
namespace dev {

void ADS1015::setup(i2c_t3 *wire, uint8_t addr, unsigned int alert_pin, unsigned long timeout) {
  this->I2CDevice::setup(wire, addr, timeout);
  this->set_sample_rate(SR::SPS_1600);
  this->set_gain(GAIN::TWO);
  this->alert_pin = alert_pin;
}

bool ADS1015::reset() {
  this->I2CDevice::reset();
  while (this->is_functional()) {
    static uint8_t const thresh[6] = {0x02, 0x00, 0x00, 0x03, 0xFF, 0xFF};
    this->i2c_begin_transmission();
    this->i2c_write(&thresh[0], 3);
    this->i2c_end_transmission(I2C_NOSTOP);
    this->i2c_begin_transmission();
    this->i2c_write(&thresh[3], 3);
    this->i2c_end_transmission(I2C_STOP);
    if (!this->i2c_pop_errors()) return true;
  }
  return false;
}

void ADS1015::set_sample_rate(SR sample_rate) {
  static unsigned int const sample_rates[7] = {128,  250,  490, 920,
                                               1600, 2400, 3300};
  this->sample_delay = (1000 / sample_rates[sample_rate >> 5]) + 2;
  this->sample_rate = sample_rate;
}

void ADS1015::start_read(CHANNEL channel) {
  // Determine the conversion start message to be sent
  uint16_t config = 0x8108 | this->sample_rate | this->gain | channel;
  uint8_t arr[] = {0x01, (uint8_t)(config >> 0x8), (uint8_t)(config & 0xFF)};
  // Wait for previous transmission to complete and transmit
  this->i2c_finish();
  this->i2c_begin_transmission();
  this->i2c_write(arr, 3);
  this->i2c_end_transmission();
  // Timestamp the conversion initialization
  this->timestamp = millis();
}

bool ADS1015::end_read(int16_t &val) {
  // Wait for alert pin or conversion timeout
  // unsigned long offset = 0;
  // if (this->timestamp + (this->sample_delay << 7) < this->timestamp)
  //  offset = (this->sample_delay << 7);
  int read;
  do {
    read = digitalRead(this->alert_pin);
  } while (read == LOW);// &&
         //millis() + offset <= this->timestamp + offset + this->sample_delay &&
         //millis() + offset > this->timestamp + offset);
  // Request conversion data
  uint8_t data[2];
  this->i2c_begin_transmission();
  this->i2c_write((uint8_t)0x00);
  this->i2c_end_transmission(I2C_NOSTOP);
  this->i2c_request_from(2);
  this->i2c_read(data, 2);
  // Check for errors
  if (this->i2c_pop_errors()) return false;
  // Format data
  uint16_t lb = data[1];
  uint16_t ub = data[0];
  val = (lb | (ub << 8)) >> 4;
  if (val & 0x0800) val |= 0xF000;
  return true;
}

bool ADS1015::read(CHANNEL channel, int16_t &val) {
  this->start_read(channel);
  return this->end_read(val);
}
}  // namespace dev
}  // namespace adcs
