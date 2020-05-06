//
// lib/GyroHeaterTransistor/GyroHeaterTransistor.cpp
// ADCS
//
// Contributors:
//   Tanishq Aggarwal (ta335@cornell.edu)
//   Shihao Cao (sfc72@cornell.edu)
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#include "GyroHeaterTransistor.hpp"

#include <Arduino.h>

namespace adcs{
namespace dev{

void GyroHeaterTransistor::setup(unsigned int pin) {
  this->pin = pin;
}

bool GyroHeaterTransistor::reset() {
  analogWrite(this->pin, 0);  
  this->Device::reset();
  return true;
}

void GyroHeaterTransistor::disable() {
  analogWrite(this->pin, 0);
  this->Device::disable();
}

void GyroHeaterTransistor::actuate(const int pwm) {
  // assuming device is on
  analogWrite(this->pin, pwm);
}

}  // namespace dev
}  // namespace adcs