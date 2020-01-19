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

Tank::Tank(size_t _num_pins) :
num_pins(_num_pins) {}

Tank1::Tank1() : Tank(2) {
    valve_lock_duration = 10*1000;
    valve_pins[0] = 27; // Main tank 1 to tank 2 valve
    valve_pins[1] = 28; // Backup tank 1 to tank 2 valve
    temp_sensor_pin = 21;
}

Tank2::Tank2() : Tank(4) {
    valve_lock_duration = 3;
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
    for (size_t i = 0; i < num_pins; ++i)
        pinMode(valve_pins[i], OUTPUT);

    pinMode(temp_sensor_pin, INPUT);
}

void Tank2::setup()
{
    Tank2::setup();
#ifndef DESKTOP
    pinMode(pressure_sensor_high_pin, INPUT);
    pinMode(pressure_sensor_high_pin, INPUT);
#endif
}

/* Tank implementation */

void Tank::reset()
{
    for (size_t i = 0; i < num_pins; ++i)
    {
        digitalWrite(valve_pins[i], LOW);
    }
}

int Tank::get_temp()
{
    return analogRead(temp_sensor_pin);
}

/* Tank2 implementation */

void Tank2::clear_schedule()
{
    set_schedule({0, 0, 0, 0});
}

void Tank2::set_schedule(const std::array<unsigned int, 4> &setting)
{
    for(size_t i = 0; i < num_pins; i++) 
        schedule[i] = setting[i];
}

float Tank2::get_pressure() {
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

/* Propulsion System implementation */

void PropulsionSystem::reset() {
    tank1.reset();
    tank2.reset();
    tank2.clear_schedule();
}

void PropulsionSystem::enable()
{
    is_enabled = true;
#ifndef DESKTOP
    tank2.thrust_valve_loop_timer.begin(thrust_valve_loop, tank2.thrust_valve_loop_interval_us);
#endif
}

void PropulsionSystem::disable() {
    is_enabled = false;
#ifndef DESKTOP
    tank2.thrust_valve_loop_timer.end();
#endif
}

bool PropulsionSystem::is_functional() { 
#ifdef DESKTOP
    return true;
#else
    return digitalRead(DCDC::dcdc_sph_enable_pin);
#endif
}

bool PropulsionSystem::open_valve(Tank& tank, size_t valve_idx)
{
    if (valve_idx >= tank.num_pins || !tank.valve_lock.is_free()) 
        return false;
    // disable();
#ifndef DESKTOP
    delayMicroseconds(10); // Wait for current cycle of the thrust valve loop to end
    tank.valve_lock.procure(tank.valve_lock_duration);
    digitalWrite(tank.valve_pins[valve_idx], HIGH);
#endif
    // enable();
    tank.is_valve_opened[valve_idx] = 1;
    return true;
}

void PropulsionSystem::close_valve(Tank& tank, size_t valve_idx)
{
    // disable();
    if (valve_idx >= tank.num_pins)
        return;
    #ifndef DESKTOP
        delayMicroseconds(10); // Wait for current cycle of the thrust valve loop to end
        digitalWrite(tank.valve_pins[valve_idx], 0);
    #endif
    tank.is_valve_opened[valve_idx] = 0;
    // enable();
}

void PropulsionSystem::thrust_valve_loop() {
    noInterrupts(); // Must disable interrupts since this function is not interrupt safe
    for (unsigned char i = 0; i < tank2.num_pins; i++) {
        if (tank2.schedule[i] < tank2.thrust_valve_loop_interval_ms) {
            // Firing on valve i - 2 is complete
            close_valve(tank2, i);
            tank2.schedule[i] = 0;
            continue;
        }
        else if (!tank2.is_valve_opened[i] && tank2.valve_lock.is_free()) {
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
