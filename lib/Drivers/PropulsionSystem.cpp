#include "SpikeAndHold.hpp"
#include <Arduino.h>
#include <algorithm>
#include <climits>
#include <tuple>
#include "../Devices/Device.hpp"
#include "DCDC.hpp"

using namespace Devices;

const std::array<unsigned char, 6>
PropulsionSystem::default_valve_pins = {27, 28, 3, 4, 5, 6};

PropulsionSystem::PropulsionSystem() : Device("propulsion") {}

bool PropulsionSystem::setup() {
    for (unsigned char i = 0; i < SpikeAndHold::num_valves; i++) {
        pinMode(valve_pins[i], OUTPUT);
    }

    pinMode(pressure_sensor_low_pin, INPUT);
    pinMode(pressure_sensor_high_pin, INPUT);
    pinMode(temp_sensor_inner_pin, INPUT);
    pinMode(temp_sensor_outer_pin, INPUT);
    return true;
}

void PropulsionSystem::disable() {
    shut_all_valves();
}

void PropulsionSystem::reset() {
    shut_all_valves();
    disable();
    delay(10);
    enable();
}

float PropulsionSystem::get_pressure() {
    static int low_gain_read = 0;
    static int high_gain_read = 0;
    static float pressure = 0;
    // set the two pressure pins as inputs

    // analog read
    low_gain_read = analogRead(pressure_sensor_low_pin);
    high_gain_read = analogRead(pressure_sensor_high_pin);

    // convert to pressure [psia]
    if (high_gain_read < 1000){
        pressure = high_gain_slope*high_gain_read + high_gain_offset;
    } else {
        pressure = low_gain_slope*low_gain_read + low_gain_offset;
    }

    return pressure;
}
signed int PropulsionSystem::get_temp_inner() {
    return 0;
}
signed int PropulsionSystem::get_temp_outer() {
    return 0;
}

bool PropulsionSystem::is_functional() { return digitalRead(DCDC::dcdc_sph_enable_pin); }

void PropulsionSystem::set_thrust_valve_state(const std::array<unsigned char, 4> &setting) {
    for (unsigned char i = 2; i < 6; i++) { digitalWrite(valve_pins[i], setting[i]); }
}

void PropulsionSystem::set_tank_valve_state(bool valve, bool state) {
    digitalWrite(valve_pins[valve], state);
}

void PropulsionSystem::shut_all_valves() {
    for (unsigned char i = 0; i < num_valves; i++) digitalWrite(valve_pins[i], OFF);
}
