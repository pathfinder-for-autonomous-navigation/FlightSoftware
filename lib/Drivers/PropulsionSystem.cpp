#include "SpikeAndHold.hpp"
#include <Arduino.h>
#include <algorithm>
#include <climits>
#include <tuple>
#include "../Devices/Device.hpp"

using namespace Devices;

const std::array<unsigned char, 6>
PropulsionSystem::default_valve_pins = {27, 28, 3, 4, 5, 6};

PropulsionSystem::PropulsionSystem() : Device("propulsion") {}

bool PropulsionSystem::setup() {
    for (unsigned char i = 0; i < SpikeAndHold::num_valves; i++) {
        pinMode(valve_pins[i], OUTPUT);
    }
    pinMode(dcdc_sph_enable_pin, OUTPUT);
    digitalWrite(dcdc_sph_enable_pin, OFF);
    is_enabled = false;

    pinMode(pressure_sensor_low_pin, INPUT);
    pinMode(pressure_sensor_high_pin, INPUT);
    pinMode(temp_sensor_inner_pin, INPUT);
    pinMode(temp_sensor_outer_pin, INPUT);
    return true;
}

void PropulsionSystem::disable() {
    shut_all_valves();
    digitalWrite(enable_pin_, OFF);
    is_enabled = false;
}

void PropulsionSystem::reset() {
    shut_all_valves();
    disable();
    delay(10);
    enable();
}

void PropulsionSystem::enable() {
    digitalWrite(enable_pin_, ON);
    is_enabled = true;
}

float PropulsionSystem::get_pressure() {
    return 0;
}
signed int PropulsionSystem::get_temp_inner() {
    return 0;
}
signed int PropulsionSystem::get_temp_outer() {
    return 0;
}

bool PropulsionSystem::is_functional() { return is_enabled; }

void PropulsionSystem::set_valves(const std::array<unsigned char, 4> &setting) {
    for (unsigned char i = 2; i < 6; i++) { digitalWrite(valve_pins[i], setting[i]); }
}

void PropulsionSystem::shut_all_valves() {
    for (unsigned char i = 0; i < num_valves; i++) digitalWrite(valve_pins[i], OFF);
}
