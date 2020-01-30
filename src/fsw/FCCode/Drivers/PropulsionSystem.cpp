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

Tank1::Tank1() : Tank(2) {
    mandatory_wait_time_ms = 10*1000; // 10 seconds
    valve_pins[0] = 27; // Main tank 1 to tank 2 valve
    valve_pins[1] = 28; // Backup tank 1 to tank 2 valve
    temp_sensor_pin = 21;
}

Tank2::Tank2() : Tank(4) {
    mandatory_wait_time_ms = 3; // 3 ms
    valve_pins[0] = 3; // Nozzle valve
    valve_pins[1] = 4; // Nozzle valve
    valve_pins[2] = 5; // Nozzle valve
    valve_pins[3] = 6; // Nozzle valve
    temp_sensor_pin = 22;
}

/** Initialize static variables */

PropulsionSystem::PropulsionSystem() : Device("propulsion") {}
volatile bool PropulsionSystem::is_enabled = 0;
Tank1 PropulsionSystem::tank1 = Tank1();
Tank2 PropulsionSystem::tank2 = Tank2();

volatile unsigned int Tank2::schedule[4] = {0, 0, 0, 0};
#ifndef DESKTOP
IntervalTimer Tank2::thrust_valve_loop_timer = IntervalTimer();
#endif
uint32_t Tank2::start_time = 0;

/* Setup */

bool PropulsionSystem::setup() {
    tank1.setup();
    tank2.setup();
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

void Tank2::setup()
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

float Tank2::get_pressure() const {
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

unsigned int Tank2::get_schedule_at(size_t valve_num) const
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
        tank1.close_all_valves();
        tank1.valve_lock.end_time = 0;
        tank2.valve_lock.end_time = 0;
        clear_schedule();
    }
    interrupts();
}

bool PropulsionSystem::enable()
{
    if (is_enabled)
    {
        disable(); // If this happens, then there's a big problem
        return false;
    }

    if ( !is_start_time_ok(tank2.start_time) )
        return false;

    // Unlock 2.9 ms from start time so as to not be one interrupt-cycle (3 ms) late
    if (tank2.valve_lock.procure(tank2.start_time - micros() - 100))
    {
#ifndef DESKTOP
        tank2.thrust_valve_loop_timer.begin(thrust_valve_loop, tank2.thrust_valve_loop_interval_us);
#endif
        is_enabled = true;
    }
    return is_enabled;
}

bool PropulsionSystem::is_start_time_ok(uint32_t start_time_us)
{
    uint32_t diff = TimedLock::safe_subtract(start_time_us, micros());
    // start_time must be at least 3000 us into the future
    if (diff <= tank2.thrust_valve_loop_interval_us)
        return false;
    return true;
}

void PropulsionSystem::disable() {
    noInterrupts();
#ifndef DESKTOP
    tank2.thrust_valve_loop_timer.end();
#endif
    interrupts();

    tank2.close_all_valves();
    is_enabled = false;

    // Release tank2 lock if tank2 had not started firing
    if (tank2.start_time > micros())
        tank2.valve_lock.end_time = 0;
}

bool PropulsionSystem::is_functional() { 
    // TODO: change this later maybe
    return true;
}

bool PropulsionSystem::set_schedule(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t start_time_us)
{

    // Do not set the schedule when enabled
    if (is_enabled)
        return false;

    // Scheduled firing must be at least 3000 us in the future
    if (!is_start_time_ok(start_time_us))
        return false;

    // Maximum allowed firing per valve is 1 second
    if (a >= 1000 || b >= 1000 || c >= 1000 || d >= 1000)
        return false;
 
    tank2.start_time = start_time_us;

    tank2.schedule[0] = (a >= tank2.min_firing_duration_ms) ? a : 0;
    tank2.schedule[1] = (b >= tank2.min_firing_duration_ms) ? b : 0;
    tank2.schedule[2] = (c >= tank2.min_firing_duration_ms) ? c : 0;
    tank2.schedule[3] = (d >= tank2.min_firing_duration_ms) ? d : 0;
    return true;
}

bool PropulsionSystem::clear_schedule()
{
    if (is_enabled)
        return false;
    for (size_t i = 0; i < 4; ++i)
        tank2.schedule[i] = 0;
    return true;
}

bool PropulsionSystem::is_done_firing() const
{
    if (!is_enabled)
        return false;
    for (size_t i = 0; i < 4; ++i)
    {
        if (tank2.schedule[i] != 0)
            return false;
    }
    return true;
}

bool PropulsionSystem::open_valve(Tank& tank, size_t valve_idx)
{
    if (valve_idx >= tank.num_valves || !tank.valve_lock.is_free()) 
        return false;
    noInterrupts();
    {
        if (tank.valve_lock.procure(tank.mandatory_wait_time_ms*1000 - 100))
        {
            digitalWrite(tank.valve_pins[valve_idx], HIGH);
            tank.is_valve_opened[valve_idx] = 1;
        }
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
    // Serial.printf("\nCurrent Time: %u\n",micros());
    for (size_t i = 0; i < tank2.num_valves; ++i) {
        // If a valve is scheduled to open for less than 3 ms, then ignore it
        if (tank2.schedule[i] < tank2.thrust_valve_loop_interval_ms) {
            if (tank2.is_valve_opened[i] || tank2.schedule[i])
            {
                // Serial.printf("Close vault %d\n", i);
                tank2.schedule[i] = 0;
                close_valve(tank2, i);
            }
            continue;
        }
        // lock will always be free since this timer interrupts on 3ms anyway
        else if (!tank2.is_valve_opened[i]) {
            // Open valve and prevent other valves from opening at the same time
            // Serial.printf("Open vault %d\n", i);
            open_valve(tank2, i);
        }
        if (tank2.is_valve_opened[i]) {
            // If the valve is open, then decrement 3 ms from its schedule
            tank2.schedule[i] -= tank2.thrust_valve_loop_interval_ms;
            // Serial.printf("Decrement vault %d to %d\n", i, tank2.schedule[i]);
        }
    }
    interrupts();
}

