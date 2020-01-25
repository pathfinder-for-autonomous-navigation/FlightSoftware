//
// src/adcs/dev/MaxonEC45.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol          kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#include "MaxonEC45.hpp"

#include <Arduino.h>

namespace adcs {
namespace dev {

void MaxonEC45::setup(unsigned int cw_pin, unsigned int ccw_pin, unsigned int speed_pin,
                      AD5254 *potentiometer, void (AD5254::*const set_r)(uint8_t)) {
  this->cw_pin = cw_pin;
  this->ccw_pin = ccw_pin;
  this->servo.attach(speed_pin);
  this->potentiometer = potentiometer;
  this->set_r = set_r;
}

bool MaxonEC45::reset() {
  this->Device::reset();
  this->set_axl_ramp(0);
  this->set_speed(1000);
  this->stop();
  return true;
}

void MaxonEC45::disable() {
  this->Device::disable();
  this->set_axl_ramp(0);
  this->set_speed(1000);
  this->stop();
}

void MaxonEC45::actuate() {
  (this->potentiometer->*this->set_r)(this->axl_ramp);
  if (this->speed >= 0) {
    digitalWrite(this->cw_pin, LOW);
    digitalWrite(this->ccw_pin, HIGH);
    servo.writeMicroseconds(speed);
  } else {
    digitalWrite(this->cw_pin, HIGH);
    digitalWrite(this->ccw_pin, LOW);
    servo.writeMicroseconds(-speed);
  }
}

void MaxonEC45::stop() {
  digitalWrite(this->cw_pin, LOW);
  digitalWrite(this->ccw_pin, LOW);
}
}  // namespace dev
}  // namespace adcs
