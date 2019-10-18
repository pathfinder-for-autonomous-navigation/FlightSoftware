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

    enable();

    return true;
}

bool DockingSystem::is_functional() {
    return digitalRead(dcdc_enable_pin) && is_enabled;
}

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

void DockingSystem::set_step_angle(float angle) {
    step_angle = angle;
}

void DockingSystem::dock() {
    if (is_turning_clockwise) {
        digitalWrite(motor_direction_pin, HIGH);
    } 
    else {
        digitalWrite(motor_direction_pin, LOW);
    }

    step_motor(180.0f);
}

void DockingSystem::undock() {
    if (is_turning_clockwise) {
        digitalWrite(motor_direction_pin, LOW);
    } 
    else {
        digitalWrite(motor_direction_pin, HIGH);
    }
    step_motor(180.0f);
}

void DockingSystem::step_motor(float angle) {
    unsigned int steps = (int)(angle/step_angle);
    unsigned int current_step = 0;

    // step at a frequency of 500Hz
    while (current_step < steps){
        digitalWrite(stepper_pin_, LOW);
        delayMicroseconds(1000);
        digitalWrite(stepper_pin_, HIGH);
        delayMicroseconds(1000);
        current_step++;
    }
}
