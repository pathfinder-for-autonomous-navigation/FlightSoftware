#ifndef DESKTOP
#include "DockingMotor.hpp"
#include "Arduino.h"

using namespace Devices;

DockingMotor::DockingMotor(const std::string &name, unsigned char i1, unsigned char i2,
                           unsigned char dir, unsigned char sleep, unsigned char step)
    : Device(name),
      i1_pin_(i1),
      i2_pin_(i2),
      direction_pin_(dir),
      sleep_pin_(sleep),
      stepper_pin_(step) {}

bool DockingMotor::setup() {
    pinMode(sleep_pin_, OUTPUT);
    pinMode(stepper_pin_, OUTPUT);
    pinMode(direction_pin_, OUTPUT);
    pinMode(i1_pin_, OUTPUT);
    pinMode(i2_pin_, OUTPUT);

    // set current limit (I2 low, I1 93.8% duty cycle)
    digitalWrite(i2_pin_, LOW);
    analogWrite(i1_pin_, 239);

    // SLEEP pins is set high to disable sleep
    digitalWrite(sleep_pin_, HIGH);

    return true;
}

bool DockingMotor::is_functional() { return true; }

void DockingMotor::disable() { digitalWrite(sleep_pin_, LOW); }

void DockingMotor::reset() {
    digitalWrite(sleep_pin_, LOW);
    delay(10);
    digitalWrite(sleep_pin_, HIGH);
}

void DockingMotor::dock() {
    if (docked_) return;
    // TODO
    docked_ = true;
}

void DockingMotor::undock() {
    if (!docked_) return;
    // TODO
    docked_ = false;
}
#endif