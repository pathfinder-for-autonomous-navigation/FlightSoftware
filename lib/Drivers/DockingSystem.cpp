#include "DockingSystem.hpp"
#include "DCDC.hpp"
#ifndef DESKTOP
#include "Arduino.h"
#endif

using namespace Devices;

DockingSystem::DockingSystem()
    : Device("docking_system") {}

bool DockingSystem::setup() {
    #ifndef DESKTOP
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
    #endif

    return true;
}

bool DockingSystem::is_functional() {
    #ifndef DESKTOP
    return digitalRead(DCDC::dcdc_sph_enable_pin)
        && digitalRead(motor_sleep_pin);
    #else
    return true;
    #endif
}

void DockingSystem::disable() { 
    #ifndef DESKTOP
    digitalWrite(motor_sleep_pin, LOW);
    #endif
}
void DockingSystem::enable() {
    #ifndef DESKTOP
    digitalWrite(motor_sleep_pin, HIGH);
    #endif
}

void DockingSystem::reset() {
    #ifndef DESKTOP
    disable();
    delay(10);
    enable();
    #endif
}

bool DockingSystem::check_docked() const {
    #ifndef DESKTOP
    return digitalRead(switch_pin);
    #else
    return is_docked;
    #endif
}

void DockingSystem::set_step_angle(float angle) {
    step_angle = angle;
}

void DockingSystem::set_direction(bool set_clockwise) {
    #ifndef DESKTOP
    if (set_clockwise) {
        digitalWrite(motor_direction_pin, LOW);
    }
    else {
        digitalWrite(motor_direction_pin, HIGH);
    }
    #endif
}

void DockingSystem::start_dock() {
    const bool clockwise_direction = true;
    set_direction(clockwise_direction);
    set_turn_angle(180.0f);
}

void DockingSystem::start_undock() {
    const bool clockwise_direction = false;
    set_direction(clockwise_direction);
    set_turn_angle(180);
}

void DockingSystem::step_motor() {
    #ifndef DESKTOP
    digitalWrite(motor_step_pin, LOW);
    delayMicroseconds(1000);
    digitalWrite(motor_step_pin, HIGH);
    #endif
    steps=steps-1;
}

float DockingSystem::get_step_angle() const {
    return step_angle;
}

void DockingSystem::set_turn_angle(float angle) {
    steps=angle/step_angle;
}

unsigned int DockingSystem::get_steps() const {
    return steps;
}

#ifdef DESKTOP
void DockingSystem::set_dock(bool dock) {
    is_docked = dock;
}
#endif
