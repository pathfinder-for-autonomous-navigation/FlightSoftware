/**
 * Implementation of PropulsionSystem.hpp
 * Changed date april 17 2020
 */
#include "PropulsionSystem.hpp"
#include <algorithm>
#include <climits>
#include <tuple>
#include <cmath>
#include "DCDC.hpp"
using namespace Devices;

#ifdef DESKTOP
/**
 * These functions and macros are defined in Arduino.h, which we cannot include
 * in DESKTOP tests
 */
static void interrupts(){}
void noInterrupts(){}
uint8_t analogRead(uint8_t pin){return 0;}
uint8_t digitalRead(uint8_t pin){return 0;}
void digitalWrite(uint8_t pin, uint8_t val){}
#define LOW 0
#define HIGH 1
#endif

/* Constructors */

Tank::Tank() {}

_Tank1::_Tank1() : Tank() {
    num_valves = 2;
    valve_pins[0] = valve_primary_pin; // Main tank 1 to tank 2 valve
    valve_pins[1] = valve_backup_pin; // Backup tank 1 to tank 2 valve
    temp_sensor_pin = tank1_temp_sensor_pin;
#ifdef DESKTOP
    p_fake_temp_read = &fake_tank1_temp_sensor_read;
#endif
}

_Tank2::_Tank2() : Tank() {
    num_valves = 4;
    valve_pins[0] = valve1_pin; // Nozzle valve
    valve_pins[1] = valve2_pin; // Nozzle valve
    valve_pins[2] = valve3_pin; // Nozzle valve
    valve_pins[3] = valve4_pin; // Nozzle valve
    temp_sensor_pin = tank2_temp_sensor_pin;
#ifdef DESKTOP
    p_fake_temp_read = &fake_tank2_temp_sensor_read;
#endif
}

/** Initialize static variables */

_PropulsionSystem::_PropulsionSystem() : Device("propulsion") {}
bool _PropulsionSystem::is_interval_enabled = 0;

volatile unsigned int _Tank2::schedule[4] = {0, 0, 0, 0};

#ifndef DESKTOP
IntervalTimer _Tank2::thrust_valve_loop_timer = IntervalTimer();
#endif

/* Setup */

bool _PropulsionSystem::setup() {
    Tank1.setup();
    Tank2.setup();
#ifndef DESKTOP
    // Set 10-bit resolution since the regression for the pressure sensor
    // calculations was computed using a 10-bit Teensy
    analogReadAveraging(10);
#endif
    return true;
}

void Tank::setup()
{
#ifndef DESKTOP
    for (size_t i = 0; i < num_valves; ++i)
        pinMode(valve_pins[i], OUTPUT);

    pinMode(temp_sensor_pin, INPUT);
#endif
}

void _Tank2::setup()
{
    Tank::setup();
#ifndef DESKTOP
    pinMode(pressure_sensor_high_pin, INPUT);
    pinMode(pressure_sensor_low_pin, INPUT);
#endif
}

/* Tank implementation */

int Tank::get_temp() const
{
    // Get the resistance of the temp sensor by measuring
    // a reference voltage and using the voltage divider equation
#ifdef DESKTOP
    // fake raw input for temperature sensor pin
    unsigned int raw = *p_fake_temp_read;
#else
    unsigned int raw = analogRead(temp_sensor_pin);
#endif
    double voltage = raw * 3.3 / 1024.0;
    if (std::abs(3.3 - voltage) < 1e-4)
        return tank_temp_min;

    double resist = (voltage * 6200.0) / (3.3 - voltage);

    // Get the value of the temperature using a linear regression
    if (std::abs(resist) <= 1)
        return tank_temp_max;
    else
        return temp_a * std::pow(std::log(resist), temp_exp) + temp_b;
}

bool Tank::is_valve_open(size_t valve_idx) const
{
    if (valve_idx >= num_valves)
        return false;
#ifdef DESKTOP
    return is_valve_opened[valve_idx];
#else
    return digitalRead(valve_pins[valve_idx]);
#endif
}

void Tank::close_all_valves()
{
    for (size_t i = 0; i < num_valves; ++i)
    {
        digitalWrite(valve_pins[i], LOW);
        is_valve_opened[i] = 0;
    }
}

/* Tank2 implementation */

float _Tank2::get_pressure() const {
    static unsigned int low_gain_read = 0;
    static unsigned int high_gain_read = 0;
    static float pressure = 0;

    // analog read
#ifdef DESKTOP
    low_gain_read = fake_tank2_pressure_low_read;
    high_gain_read = fake_tank2_pressure_high_read;
#else
    low_gain_read = analogRead(pressure_sensor_low_pin);
    high_gain_read = analogRead(pressure_sensor_high_pin);
#endif

    // convert to pressure [psia]
    if (high_gain_read < amp_threshold){
        pressure = high_gain_slope*high_gain_read + high_gain_offset;
    } else {
        pressure = low_gain_slope*low_gain_read + low_gain_offset;
    }

    return pressure;
}

unsigned int _Tank2::get_schedule_at(size_t valve_num) const
{
    if (valve_num >= num_valves)
        return 0;
    return schedule[valve_num];
}

/* Propulsion System implementation */

void _PropulsionSystem::reset() {
    noInterrupts();
    {    
        disable();
        Tank1.close_all_valves();
        clear_schedule();
    }
    interrupts();
}

bool _PropulsionSystem::start_firing()
{
    if (is_interval_enabled)
    {
        disable(); // If this happens, then there's a big problem
        return false;
    }
#ifndef DESKTOP
        Tank2.thrust_valve_loop_timer.begin(thrust_valve_loop, Tank2.thrust_valve_loop_interval_ms*1000);
#endif
        is_interval_enabled = true;
    return is_interval_enabled;
}

void _PropulsionSystem::disable() {
    noInterrupts();
#ifndef DESKTOP
    Tank2.thrust_valve_loop_timer.end();
#endif
    interrupts();

    Tank2.close_all_valves();
    is_interval_enabled = false;

}

bool _PropulsionSystem::is_functional() { 
    return digitalRead(DCDC::SpikeDockDCDC_EN) == HIGH;
}

bool _PropulsionSystem::set_schedule(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{

    // Do not set the schedule when enabled
    if (is_interval_enabled)
        return false;

    // Maximum allowed firing per valve is 1 second
    if (a >= 1000 || b >= 1000 || c >= 1000 || d >= 1000)
        return false;

    Tank2.schedule[0] = (a >= Tank2.min_firing_duration_ms) ? a : 0;
    Tank2.schedule[1] = (b >= Tank2.min_firing_duration_ms) ? b : 0;
    Tank2.schedule[2] = (c >= Tank2.min_firing_duration_ms) ? c : 0;
    Tank2.schedule[3] = (d >= Tank2.min_firing_duration_ms) ? d : 0;
    return true;
}

bool _PropulsionSystem::clear_schedule()
{
    if (is_interval_enabled)
        return false;
    for (size_t i = 0; i < 4; ++i)
        Tank2.schedule[i] = 0;
    return true;
}


bool _PropulsionSystem::open_valve(Tank& tank, size_t valve_idx)
{
    if (valve_idx >= tank.num_valves ) 
        return false;
    noInterrupts();
    {
        digitalWrite(tank.valve_pins[valve_idx], HIGH);
        tank.is_valve_opened[valve_idx] = 1;
    }
    interrupts();
    return true;
}

void _PropulsionSystem::close_valve(Tank& tank, size_t valve_idx)
{
    if (valve_idx >= tank.num_valves)
        return;
    noInterrupts();
    {
        digitalWrite(tank.valve_pins[valve_idx], LOW);
        tank.is_valve_opened[valve_idx] = 0;
    }
    interrupts();
}

void _PropulsionSystem::thrust_valve_loop() {
    noInterrupts(); 
    bool did_open_valve = 0;
    // Serial.printf("\nCurrent Time: %u\n",micros());
    for (size_t i = 0; i < Tank2.num_valves; ++i) {
        // If a valve is scheduled to open for less than 3 ms, then ignore it
        if (Tank2.schedule[i] < Tank2.thrust_valve_loop_interval_ms) {
            if (Tank2.is_valve_opened[i] || Tank2.schedule[i])
            {
                // Serial.printf("Close vault %d\n", i);
                Tank2.schedule[i] = 0;
                close_valve(Tank2, i);
            }
            continue;
        }
        // Only open a valve if we did not open a valve at this interrupt cycle
        else if (!Tank2.is_valve_opened[i] && !did_open_valve) {
            // Open valve and prevent other valves from opening at the same time
            // Serial.printf("Open vault %d\n", i);
            did_open_valve = 1;
            open_valve(Tank2, i);
        }
        if (Tank2.is_valve_opened[i]) {
            // If the valve is open, then decrement 3 ms from its schedule
            Tank2.schedule[i] -= Tank2.thrust_valve_loop_interval_ms;
            // Serial.printf("Decrement vault %d to %d\n", i, Tank2.schedule[i]);
        }
    }
    interrupts();
}

