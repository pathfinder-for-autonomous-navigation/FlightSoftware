#include "DockingMotor.hpp"
#include "Arduino.h"

using namespace Devices;

DockingSystem::DockingSystem(const std::string &name, unsigned char i1, unsigned char i2,
                           unsigned char dir, unsigned char sleep, unsigned char step)
    : Device("docking_system"), rotation_parameter(0), is_enabled(false) {}

bool DockingSystem::setup() {
    pinMode(motor_sleep_pin, OUTPUT);
    pinMode(motor_step_pin, OUTPUT);
    pinMode(motor_direction_pin, OUTPUT);
    pinMode(motor_i1_pin, OUTPUT);
    pinMode(motor_i2_pin, OUTPUT);
    pinMode(switch_pin, INPUT);

    // set current limit (I2 low, I1 93.8% duty cycle)
    digitalWrite(motor_i2_pin, LOW);
    analogWrite(motor_i1_pin, 239);

    // SLEEP pins is set low to enable sleep
    digitalWrite(motor_sleep_pin, LOW);

    return true;
}

bool DockingSystem::is_functional() { return true; }

void DockingSystem::disable() { 
    digitalWrite(motor_sleep_pin, LOW); 
    is_enabled = false;
}
void DockingSystem::enable() {
    digitalWrite(motor_sleep_pin, HIGH);
    is_enabled = true;
}

void DockingSystem::reset() {
    disable();
    delay(10);
    enable();
}

bool DockingSystem::check_docked() const {
    return digitalRead(switch_pin);
}

void DockingSystem::set_rotation_parameter(unsigned int parameter) {
    rotation_parameter = parameter;
}

void DockingSystem::dock() {
    // TODO
}

void DockingSystem::undock() {
    // TODO
}
