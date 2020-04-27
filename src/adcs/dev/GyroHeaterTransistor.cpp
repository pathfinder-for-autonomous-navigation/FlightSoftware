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
  digitalWrite(pin, LOW);
  return true;
}

void GyroHeaterTransistor::disable() {
  digitalWrite(pin, LOW);
}

void GyroHeaterTransistor::actuate() {
  digitalWrite(pin, HIGH);
}

}  // namespace dev
}  // namespace adcs