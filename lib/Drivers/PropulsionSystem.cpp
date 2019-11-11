#include "PropulsionSystem.hpp"
#ifndef DESKTOP
#include <Arduino.h>
#endif

#include <algorithm>
#include <climits>
#include <tuple>
#include "../Devices/Device.hpp"
#include "DCDC.hpp"

using namespace Devices;

const std::array<unsigned char, 6>
PropulsionSystem::valve_pins = {27, 28, 3, 4, 5, 6};

volatile bool PropulsionSystem::valve_start_locked_out = false;

PropulsionSystem::PropulsionSystem() : Device("propulsion") {}

bool PropulsionSystem::setup() {
#ifdef DESKTOP
// TODO
#else
    for (unsigned char i = 0; i < 6; i++) {
        pinMode(valve_pins[i], OUTPUT);
    }

    pinMode(pressure_sensor_low_pin, INPUT);
    pinMode(pressure_sensor_high_pin, INPUT);
    pinMode(temp_sensor_inner_pin, INPUT);
    pinMode(temp_sensor_outer_pin, INPUT);

    thrust_valve_loop_timer.begin(thrust_valve_loop, thrust_valve_loop_interval_us);
#endif
    return true;
}

void PropulsionSystem::disable() {
#ifdef DESKTOP
// TODO
#else
    for (unsigned char i = 0; i < 4; i++) thrust_valve_schedule[i] = 0;
    for (unsigned char i = 0; i < 6; i++) digitalWrite(valve_pins[i], LOW);
    thrust_valve_loop_timer.end();
#endif
}

void PropulsionSystem::enable() {
#ifdef DESKTOP
// TODO
#else
    thrust_valve_loop_timer.begin(thrust_valve_loop, thrust_valve_loop_interval_us);
#endif
}

void PropulsionSystem::reset() {
    disable();
#ifndef DESKTOP
    delay(10);
#endif
    enable();
}

float PropulsionSystem::get_pressure() {
    static int low_gain_read = 0;
    static int high_gain_read = 0;
    static float pressure = 0;
    // set the two pressure pins as inputs
    
#ifdef DESKTOP
    low_gain_read = 2; // TODO fix
    high_gain_read = 2;
#else
    // analog read
    low_gain_read = analogRead(pressure_sensor_low_pin);
    high_gain_read = analogRead(pressure_sensor_high_pin);
#endif

    // convert to pressure [psia]
    if (high_gain_read < 1000){
        pressure = high_gain_slope*high_gain_read + high_gain_offset;
    } else {
        pressure = low_gain_slope*low_gain_read + low_gain_offset;
    }

    return pressure;
}

signed int PropulsionSystem::get_temp_inner() {
#ifdef DESKTOP
    return 0;
#else
    return 0; // TODO replace with analogRead
#endif
}

signed int PropulsionSystem::get_temp_outer() {
#ifdef DESKTOP
    return 0;
#else
    return 0; // TODO replace with analogRead
#endif
}

bool PropulsionSystem::is_functional() { 
#ifdef DESKTOP
    return true;
#else
    return digitalRead(DCDC::dcdc_sph_enable_pin);
#endif
}

void PropulsionSystem::set_thrust_valve_schedule(const std::array<unsigned int, 4> &setting) {
    disable();
#ifndef DESKTOP
    delayMicroseconds(10); // Wait for current cycle of the thrust valve loop to end
#endif
    for(size_t i = 0; i < 4; i++) thrust_valve_schedule[i] = setting[i];
    enable();
}

void PropulsionSystem::set_tank_valve_state(bool valve, bool state) {
    disable();
#ifndef DESKTOP
    delayMicroseconds(10); // Wait for current cycle of the thrust valve loop to end
    digitalWrite(valve_pins[valve], state);
#endif
    enable();
}

void PropulsionSystem::thrust_valve_loop() {
    for (unsigned char i = 2; i < 6; i++) {
        if (thrust_valve_schedule[i - 2] < thrust_valve_loop_interval_ms) {
            // Firing on valve i - 2 is complete
            #ifndef DESKTOP
            digitalWrite(valve_pins[i - 2], LOW);
            #endif

            thrust_valve_schedule[i - 2] = 0;
            is_valve_opened[i] = false;
            continue;
        }
        else if (!is_valve_opened[i] && !valve_start_locked_out) {
            // Open valve and prevent other valves from opening at the same time
            valve_start_locked_out = true;
            #ifndef DESKTOP
            digitalWrite(valve_pins[i - 2], HIGH);
            #endif
            is_valve_opened[i] = true;
        }
        else if (valve_start_locked_out) {
            // 2 ms have passed since the lockout happened, so it's safe to unlock
            // the valves.
            valve_start_locked_out = false;
        }

        if (is_valve_opened[i]) {
            // Decrement the timer for the valve being open
            thrust_valve_schedule[i - 2] -= thrust_valve_loop_interval_ms;
        }
    }
}
