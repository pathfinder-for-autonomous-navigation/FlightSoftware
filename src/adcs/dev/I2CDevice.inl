//
// src/adcs/dev/I2CDevice.inl
// FlightSoftware
//
// Contributors:
//   Kyle Krol         kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#include "I2CDevice.hpp"

namespace adcs {
namespace dev {

inline bool I2CDevice::reset() {
  this->Device::reset();
  this->error_count = 0;
  this->error_acc = 0;
  return true;
}

inline I2CDevice::I2CDevice() : Device() { }

inline void I2CDevice::setup(i2c_t3 *wire, uint8_t addr, unsigned long timeout) {
  this->wire = wire;
  this->addr = addr;
  this->timeout = timeout;
  this->I2CDevice::reset();
}

inline bool I2CDevice::i2c_peek_errors() {
  return (this->error_acc != 0);
}

inline bool I2CDevice::i2c_pop_errors() {
  if (this->i2c_peek_errors())
    if (++this->error_count >= DEV_I2C_ERROR_COUNT) 
      this->Device::disable(); //don't attempt further I2C comms
  bool temp = this->i2c_peek_errors();
  if (!temp) this->error_count = 0;
  this->error_acc = 0;
  return temp;
}

inline void I2CDevice::i2c_begin_transmission() {
  this->wire->beginTransmission(this->addr);
}

inline void I2CDevice::i2c_end_transmission(i2c_stop s) {
  bool err = (this->wire->endTransmission(s, this->timeout) != 0);
  this->error_acc = (this->error_acc || err);
}

inline void I2CDevice::i2c_send_transmission(i2c_stop s) {
  this->wire->sendTransmission(s);
}

inline void I2CDevice::i2c_request_from(unsigned int len, i2c_stop s) {
  bool err = (this->wire->requestFrom(this->addr, len, s, this->timeout) != len);
  this->error_acc = (this->error_acc || err);
}

inline void I2CDevice::i2c_send_request(unsigned int len, i2c_stop s) {
  this->wire->sendRequest(this->addr, len, s);
}

inline bool I2CDevice::i2c_done() const {
  return (this->wire->done() == 1);
}

inline void I2CDevice::i2c_finish() {
  bool err = (this->wire->finish(this->timeout) == 0);
  this->error_acc = (this->error_acc || err);
}

inline void I2CDevice::i2c_write(uint8_t data) {
  bool err = (this->wire->write(data) != 1);
  this->error_acc = (this->error_acc || err);
}

template <typename T>
inline void I2CDevice::i2c_write(T const *data, unsigned int len) {
  bool err = (this->wire->write((uint8_t *)data, len * sizeof(T)) != len * sizeof(T));
  this->error_acc = (this->error_acc || err);
}

inline uint8_t I2CDevice::i2c_read() {
  int val = this->wire->read();
  this->error_acc = (this->error_acc || (val == -1));
  return (uint8_t) (*((unsigned int *) &val));
}

template <typename T>
inline void I2CDevice::i2c_read(T *data, unsigned int len) {
  bool err = (this->wire->read((uint8_t *)data, len * sizeof(T)) != len * sizeof(T));
  this->error_acc = (this->error_acc || err);
}
}  // namespace dev
}  // namespace adcs
