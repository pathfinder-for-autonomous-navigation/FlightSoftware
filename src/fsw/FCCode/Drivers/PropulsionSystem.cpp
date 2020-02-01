/**
 * Implementation of PropulsionSystem.hpp
 */
#include "PropulsionSystem.hpp"
#include <algorithm>
#include <climits>
#include <tuple>
#include "DCDC.hpp"
using namespace Devices;

#ifdef DESKTOP
/**
 * These functions and macros are defined in Arduino.h, which we cannot include
 * in DESKTOP tests
 */
static void interrupts(){}
void noInterrupts(){}
uint8_t analogRead(uint8_t pin){return pin%2;}
uint8_t digitalRead(uint8_t pin){return pin%2;}
void digitalWrite(uint8_t pin, uint8_t val){}
#define LOW 0
#define HIGH 1
#endif

/* Constructors */

Tank::Tank(size_t _num_valves) :
num_valves(_num_valves) {}

_Tank1::_Tank1() : Tank(2) {
    valve_pins[0] = 27; // Main tank 1 to tank 2 valve
    valve_pins[1] = 28; // Backup tank 1 to tank 2 valve
    temp_sensor_pin = 21;
}

_Tank2::_Tank2() : Tank(4) {
    valve_pins[0] = 3; // Nozzle valve
    valve_pins[1] = 4; // Nozzle valve
    valve_pins[2] = 5; // Nozzle valve
    valve_pins[3] = 6; // Nozzle valve
    temp_sensor_pin = 22;
}

/** Initialize static variables */

PropulsionSystem::PropulsionSystem() : Device("propulsion") {}
volatile bool PropulsionSystem::is_interval_enabled = 0;

volatile unsigned int _Tank2::schedule[4] = {0, 0, 0, 0};

#ifndef DESKTOP
IntervalTimer _Tank2::thrust_valve_loop_timer = IntervalTimer();
#endif

/* Setup */

bool PropulsionSystem::setup() {
    Tank1.setup();
    Tank2.setup();
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
    // TODO
    return analogRead(temp_sensor_pin);
}

bool Tank::is_valve_open(size_t valve_idx) const
{
    if (valve_idx >= num_valves)
        return false;
    return digitalRead(valve_pins[valve_idx]);
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
    // TODO
    static int low_gain_read = 0;
    static int high_gain_read = 0;
    static float pressure = 0;

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

unsigned int _Tank2::get_schedule_at(size_t valve_num) const
{
    if (valve_num >= num_valves)
        return 0;
    return schedule[valve_num];
}

/* Propulsion System implementation */

void PropulsionSystem::reset() {
    noInterrupts();
    {    
        disable();
        Tank1.close_all_valves();
        clear_schedule();
    }
    interrupts();
}

bool PropulsionSystem::start_firing()
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

void PropulsionSystem::disable() {
    noInterrupts();
#ifndef DESKTOP
    Tank2.thrust_valve_loop_timer.end();
#endif
    interrupts();

    Tank2.close_all_valves();
    is_interval_enabled = false;

}

bool PropulsionSystem::is_functional() { 
    // TODO: change this later maybe
    return true;
}

bool PropulsionSystem::set_schedule(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
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

bool PropulsionSystem::clear_schedule()
{
    if (is_interval_enabled)
        return false;
    for (size_t i = 0; i < 4; ++i)
        Tank2.schedule[i] = 0;
    return true;
}


bool PropulsionSystem::open_valve(Tank& tank, size_t valve_idx)
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

void PropulsionSystem::close_valve(Tank& tank, size_t valve_idx)
{
    if (valve_idx >= tank.num_valves)
        return;
    noInterrupts();
    {
        digitalWrite(tank.valve_pins[valve_idx], 0);
        tank.is_valve_opened[valve_idx] = 0;
    }
    interrupts();
}

void PropulsionSystem::thrust_valve_loop() {
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

