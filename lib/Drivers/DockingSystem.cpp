#include "DockingSystem.hpp"
#include "DCDC.hpp"
#include "Arduino.h"

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

void DockingSystem::start_dock() {
    digitalWrite(motor_direction_pin, LOW);
    set_turn_angle(180.0f);
}

void DockingSystem::start_undock() {
    digitalWrite(motor_direction_pin, HIGH);
    set_turn_angle(180);
}

void DockingSystem::step_motor() {
    digitalWrite(motor_step_pin, LOW);
    delayMicroseconds(1000);
    digitalWrite(motor_step_pin, HIGH);
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
