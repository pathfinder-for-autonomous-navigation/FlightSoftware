#include "DockingSystem.hpp"
#include "DCDC.hpp"
#ifndef DESKTOP
#include "Arduino.h"
#endif

using namespace Devices;

volatile unsigned int DockingSystem::steps = 0;

#ifndef DESKTOP
IntervalTimer timer;
#endif

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
    #endif

    return true;
}

bool DockingSystem::is_functional() {
    #ifndef DESKTOP
    return digitalRead(DCDC::SpikeDockDCDC_EN)
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

void DockingSystem::set_step_delay(unsigned int delay) {
    step_delay = delay;
}

void DockingSystem::start_halfturn() {
    set_turn_angle(180.0f);
    enable();
    #ifndef DESKTOP
    timer.begin(step_motor, step_delay);
    #endif
}

void DockingSystem::cancel() {
    disable();
    #ifndef DESKTOP
    timer.end();
    #endif
}

volatile bool motor_on = false;
void DockingSystem::step_motor() {
    if (steps >= 1) {
        if (motor_on) {
            #ifndef DESKTOP
            digitalWrite(motor_step_pin, LOW);
            #endif
            motor_on = false;
        }
        else {
            #ifndef DESKTOP
            digitalWrite(motor_step_pin, HIGH);
            #endif
            steps=steps-1;
            motor_on = true;
        }
    }
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
