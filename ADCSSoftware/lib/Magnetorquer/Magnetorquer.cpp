//
// lib/MTR/MTR.cpp
// ADCS
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#include "Magnetorquer.hpp"
#include <Arduino.h>

void Magnetorquer::setup(unsigned int f_pin, unsigned int r_pin) {
  this->f_pin = f_pin;
  this->r_pin = r_pin;
}

bool Magnetorquer::reset() {
  this->dev::Device::reset();
  this->actuate(0);
  return true;
}

void Magnetorquer::disable() {
  this->dev::Device::disable();
  this->actuate(0);
}

void Magnetorquer::actuate(int signed_pwm) {
  if (signed_pwm >= 0) {
    analogWrite(f_pin, signed_pwm);
    analogWrite(r_pin, 0);
  } else {
    analogWrite(f_pin, 0);
    analogWrite(r_pin, -signed_pwm);
  }
}
