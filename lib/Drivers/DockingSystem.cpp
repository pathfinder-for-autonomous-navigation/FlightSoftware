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

void DockingSystem::startDock() {
    if (is_turning_clockwise) {
        digitalWrite(motor_direction_pin, HIGH);
    } 
    else {
        digitalWrite(motor_direction_pin, LOW);
    }

    //step_motor(180.0f);
}

bool DockingSystem::endDock() {
    return digitalRead(switch_pin);
}

void DockingSystem::startUndock() {
    if (is_turning_clockwise) {
        digitalWrite(motor_direction_pin, LOW);
    } 
    else {
        digitalWrite(motor_direction_pin, HIGH);
    }
    //step_motor(180.0f);
}

bool DockingSystem::endUndock() {
    return digitalRead(switch_pin);
}

bool DockingSystem::check_turning(){
    return is_turning_clockwise;
}

void DockingSystem::step_motor(float angle) {
    digitalWrite(motor_step_pin, LOW);
    delayMicroseconds(1000);
    digitalWrite(motor_step_pin, HIGH);
    delayMicroseconds(1000);
}

float DockingSystem::get_step_angle(){
    return step_angle;
}

void DockingSystem::set_steps(float angle){
    steps=angle/step_angle;
}

float DockingSystem::get_steps(){
    return steps;
}