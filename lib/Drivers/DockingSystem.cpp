#include "DockingMotor.hpp"
#include "DCDC.hpp"
#include "Arduino.h"

using namespace Devices;

DockingSystem::DockingSystem()
    : Device("docking_system") {}

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
    return digitalRead(DCDC::dcdc_sph_enable_pin)
        && digitalRead(motor_sleep_pin);
}

void DockingSystem::disable() { 
    digitalWrite(motor_sleep_pin, LOW);
}
void DockingSystem::enable() {
    digitalWrite(motor_sleep_pin, HIGH);
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
