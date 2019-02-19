#include "DockingMotor.hpp"
#include "Arduino.h"

using namespace Devices;

DockingMotor::DockingMotor(unsigned char i1, 
                           unsigned char i2, 
                           unsigned char dir, 
                           unsigned char sleep, 
                           unsigned char reset, 
                           unsigned char step) : i1_pin_(i1),
                                                 i2_pin_(i2),
                                                 direction_pin_(dir),
                                                 sleep_pin_(sleep),
                                                 reset_pin_(reset),
                                                 stepper_pin_(step) {}

bool DockingMotor::setup() { 
    pinMode(reset_pin_, OUTPUT);
    pinMode(sleep_pin_, OUTPUT);
    pinMode(stepper_pin_, OUTPUT);
    pinMode(direction_pin_, OUTPUT);
    pinMode(i1_pin_, OUTPUT);
    pinMode(i2_pin_, OUTPUT);

    // set current limit (I2 low, I1 93.8% duty cycle) 
    digitalWrite(i2_pin_, LOW);
    analogWrite(i1_pin_, 239);

    // both RESET and SLEEP pins are active 
    digitalWrite(reset_pin_, HIGH);
    digitalWrite(sleep_pin_, HIGH);

    return true;
}

bool DockingMotor::is_functional() { return true; }

void DockingMotor::disable() { 
    digitalWrite(sleep_pin_, LOW);
}

void DockingMotor::reset() {
    digitalWrite(sleep_pin_, LOW);
    delay(10);
    digitalWrite(sleep_pin_, HIGH);
}

void DockingMotor::single_comp_test() { }

static std::string dockingmotor_name = "Docking Motor";
std::string& DockingMotor::name() const { return dockingmotor_name; }