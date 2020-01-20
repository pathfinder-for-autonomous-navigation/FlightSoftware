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

/* Constructors */

Tank::Tank(size_t _num_valves) :
num_valves(_num_valves) {}

Tank1::Tank1() : Tank(2) {
    valve_lock_duration = 10*1000; // 10 seconds
    valve_pins[0] = 27; // Main tank 1 to tank 2 valve
    valve_pins[1] = 28; // Backup tank 1 to tank 2 valve
    temp_sensor_pin = 21;
}

Tank2::Tank2() : Tank(4) {
    valve_lock_duration = 3; // 3 ms
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
IntervalTimer Tank2::thrust_valve_loop_timer = IntervalTimer();
uint32_t Tank2::start_time = 0;

/* Setup */

bool PropulsionSystem::setup() {
#ifndef DESKTOP
    tank1.setup();
    tank2.setup();
#endif
    return true;
}

void Tank::setup()
{
    for (size_t i = 0; i < num_valves; ++i)
        pinMode(valve_pins[i], OUTPUT);

#ifdef REAL_HARDWARE
    pinMode(temp_sensor_pin, INPUT);
#endif
}

void Tank2::setup()
{
    Tank::setup();
#ifdef REAL_HARDWARE
    pinMode(pressure_sensor_high_pin, INPUT);
    pinMode(pressure_sensor_high_pin, INPUT);
#endif
}

/* Tank implementation */

void Tank::reset()
{
    for (size_t i = 0; i < num_valves; ++i)
    {
        digitalWrite(valve_pins[i], LOW);
    }
}

int Tank::get_temp()
{
#ifdef REAL_HARDWARE
    return analogRead(temp_sensor_pin);
#else
    return 1;
#endif
}

bool Tank::is_valve_open(size_t valve_idx)
{
    if (valve_idx >= num_valves)
        return false;
    return digitalRead(valve_pins[valve_idx]);
}

bool Tank::is_lock_free()
{
    return valve_lock.is_free();
}

bool PropulsionSystem::set_schedule(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t start_time_us)
{

    // Do not set the schedule when enabled
    if (is_enabled)
        return false;

    // Scheduled firing must be in the future
    if (static_cast<int>(start_time_us - micros()) <= 0)
        return false;

    // Maximum allowed firing per valve is 1 second
    if (a >= 1000 || b >= 1000 || c >= 1000 || d >= 1000)
        return false;
 
    tank2.start_time = start_time_us;

    tank2.schedule[0] = a;
    tank2.schedule[1] = b;
    tank2.schedule[2] = c;
    tank2.schedule[3] = d;
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

/* Tank2 implementation */

float Tank2::get_pressure() {
    static int low_gain_read = 0;
    static int high_gain_read = 0;
    static float pressure = 0;

#ifndef REAL_HARDWARE
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

/* Propulsion System implementation */

void PropulsionSystem::reset() {
    // TODO: should we allow reset() while the valves are opened?
    disable();
    tank1.reset();
    tank2.reset();
    clear_schedule();
}

bool PropulsionSystem::enable()
{
#ifndef DESKTOP
    noInterrupts();
    // TODO:: assert that we are not procuring something weird
    if (tank2.valve_lock.procure(tank2.start_time - micros() - tank2.thrust_valve_loop_interval_us))
    {
        tank2.thrust_valve_loop_timer.begin(thrust_valve_loop, tank2.thrust_valve_loop_interval_us);
        is_enabled = true;
    }
   interrupts();
#endif
    return is_enabled;
}

void PropulsionSystem::disable() {
    is_enabled = false;
#ifndef DESKTOP
    noInterrupts();
    tank2.thrust_valve_loop_timer.end();
    interrupts();
    clear_schedule();
#endif
}

bool PropulsionSystem::is_functional() { 
#ifdef REAL_HARDWARE
    return digitalRead(DCDC::dcdc_sph_enable_pin);
#else
    return true;
#endif
}

bool PropulsionSystem::open_valve(Tank& tank, size_t valve_idx)
{
    if (valve_idx >= tank.num_valves || !tank.valve_lock.is_free()) 
        return false;
#ifndef DESKTOP
    noInterrupts();
    {
        // delayMicroseconds(10); // Wait for current cycle of the thrust valve loop to end
        tank.valve_lock.procure(tank.valve_lock_duration);
        digitalWrite(tank.valve_pins[valve_idx], HIGH);
        tank.is_valve_opened[valve_idx] = 1;
    }
    interrupts();
#endif
    tank.is_valve_opened[valve_idx] = 1;
    return true;
}

void PropulsionSystem::close_valve(Tank& tank, size_t valve_idx)
{
    if (valve_idx >= tank.num_valves)
        return;
    #ifndef DESKTOP
    noInterrupts();
    {
        // delayMicroseconds(10); // Wait for current cycle of the thrust valve loop to end
        digitalWrite(tank.valve_pins[valve_idx], 0);
        tank.is_valve_opened[valve_idx] = 0;
    }
    interrupts();
    #endif
}

// I assume that PropulsionController will enable interrupts when we are 3 control cycles
// away from start_time
void PropulsionSystem::thrust_valve_loop() {
    noInterrupts(); // Must disable interrupts since this function is not interrupt safe
    for (size_t i = 0; i < tank2.num_valves; ++i) {
        // If a valve is scheduled to open for less than 3 ms, then ignore it
        if (tank2.schedule[i] < tank2.thrust_valve_loop_interval_ms) {
            close_valve(tank2, i);
            tank2.schedule[i] = 0;
            continue;
        }
        else if (tank2.valve_lock.is_free() && !tank2.is_valve_opened[i]) {
            // Open valve and prevent other valves from opening at the same time
            open_valve(tank2, i);
        }
        if (tank2.is_valve_opened[i]) {
            // Decrement the timer for the valve being open
            tank2.schedule[i] -= tank2.thrust_valve_loop_interval_ms;
        }
    }
    interrupts();
}

