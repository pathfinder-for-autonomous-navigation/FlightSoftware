#include <unity.h>
#define PROP_TEST
#include <PropulsionSystem.hpp>
#include "core_pins.h"
#include "Arduino.h"
#include "usb_serial.h"

/**
 * test_prop.cpp
 * Hardware test for the Propulsion System
 */ 
#define ASSERT_TRUE(x, msg){\
UNITY_TEST_ASSERT(!x, __LINE__, msg);}

void check_all_valves_closed();
void is_schedule_expected(const std::array<unsigned int, 4> &expected_schedule);
void reset_tank();
void fill_tank(); 
void thrust();
bool is_at_threshold_pressure();
bool is_valve_open(size_t valve_pin);

const std::array<unsigned int, 4> zero_schedule = {0, 0, 0, 0};

Devices::PropulsionSystem prop_system;

void set_firing_schedule(unsigned int a, unsigned int b, unsigned int c, unsigned int d)
{
    prop_system.thrust_valve_schedule[0] = a;
    prop_system.thrust_valve_schedule[1] = b;
    prop_system.thrust_valve_schedule[2] = c;
    prop_system.thrust_valve_schedule[3] = d;
}

void test_initialization()
{
    // schedule should be initialized to 0
    is_schedule_expected(zero_schedule);
    // all valves should be closed
    check_all_valves_closed();
    // there should be no interrupts at startup
    TEST_ASSERT_FALSE(prop_system.is_enabled);
    // prop system should still be considered functional
    TEST_ASSERT_TRUE(prop_system.is_functional());
}

// Make sure that the current schedule matches the provided schedule
void is_schedule_expected(const std::array<unsigned int, 4> &expected_schedule)
{
    for (size_t i = 0; i < 4; ++i)
        TEST_ASSERT_EQUAL(expected_schedule[i], prop_system.thrust_valve_schedule);
}

// Make sure all pins low
void check_all_valves_closed()
{
    for (size_t i = 0; i < 6; ++i)
        TEST_ASSERT_EQUAL(LOW, is_valve_open(i));
}

// True if specified valve is open
bool is_valve_open(size_t valve_pin)
{
    return digitalRead(valve_pin);
}

void test_is_functional()
{
    TEST_ASSERT_TRUE(prop_system.is_functional());
}

// test that we can turn on the timer
void test_enable()
{
    // TODO: how to check that interval timer is on?
    TEST_ASSERT_TRUE(prop_system.is_functional());
    set_firing_schedule(0, 0, 0, 0);
    prop_system.enable();
    TEST_ASSERT_TRUE(prop_system.is_enabled);
    TEST_ASSERT_TRUE(prop_system.is_functional());
    check_all_valves_closed();
    TEST_ASSERT_FALSE(prop_system.valve_start_locked_out);
    delay(6); // make sure the interrupt didn't turn on the valves
    // enable should not open any valves since schedule is 0
    check_all_valves_closed();
    TEST_ASSERT_FALSE(prop_system.valve_start_locked_out);
}

// should disable interval and turn off all thrust valves
void test_disable()
{
    prop_system.disable();
    // TODO: how to check that intervaltimer is off
    TEST_ASSERT_FALSE(prop_system.is_enabled);
    TEST_ASSERT_TRUE(prop_system.is_functional());
    // check that schedule is 0 and valves are all off
    is_schedule_expected(zero_schedule);
    check_all_valves_closed();
}

// test that we can open and close the tank valves
void test_set_tank_valve_state()
{
    prop_system.set_tank_valve_state(0, 1); // open valve 0
    ASSERT_TRUE(is_valve_open(0), "make sure valve 0 is opened");
    delay(1000); // fire for 1 second
    prop_system.set_tank_valve_state(0, 0); // open valve 1
    TEST_ASSERT_FALSE(is_valve_open(0)); // make sure it is closed
}

void test_tank_lock()
{
    prop_system.set_tank_valve_state(1, 1); // open valve
    ASSERT_TRUE(prop_system.tank_valve_locked_out, "tank valves should be locked");
    delay(1000);
    prop_system.set_tank_valve_state(1, 0); // close valve
    ASSERT_TRUE (prop_system.tank_valve_locked_out, "tank should still be locked");
    delay(1000* 5);
    ASSERT_TRUE (prop_system.tank_valve_locked_out, "tank should still be locked");
    delay(1000*5); 
    ASSERT_TRUE (prop_system.tank_valve_locked_out, "tank should be unlocked");
}

// Open tank valve for 1 second and wait 10 s to make sure no one can fire
void fire_tank_valve(size_t valve_pin)
{
    if (valve_pin > 2)
    {
        TEST_ASSERT_FALSE(1);
    }
    while (prop_system.tank_valve_locked_out) // busy wait this for now
    {
        delay(140);
    }
    prop_system.set_tank_valve_state(valve_pin, 1); // open valve
    delay(1000);
    prop_system.set_tank_valve_state(valve_pin, 0); // close valve
}

// TODO: not sure how to test this
void test_set_thrust_valve_schedule()
{
    TEST_ASSERT_FALSE(prop_system.is_enabled);
    if (prop_system.get_pressure() == 0)
    {
        fill_tank();
    }
    static const std::array<unsigned int, 4> schedule = {4, 2, 3, 4};
    prop_system.set_thrust_valve_schedule(schedule);
    prop_system.enable();
    TEST_ASSERT_TRUE(is_valve_open(2) && is_valve_open(3) && is_valve_open(4) && is_valve_open(5))
}

// Fill tank to threshold value
void fill_tank()
{
    // Fire a maximum of 20 times for 1 s every 10s until we have threshold_presure
    for ( size_t i = 0; is_at_threshold_pressure() && i < 20; ++i)
    {
        prop_system.set_tank_valve_state(0, 1);
        delay(1000);
        prop_system.set_tank_valve_state(0, 0);
        delay(1000* 10); // fire every 10 seconds
    }
}

bool is_at_threshold_pressure()
{
    return abs(prop_system.get_pressure() - prop_system.threshold_pressure) > prop_system.pressure_delta;
}

// Empty out the outer tank so that pressure is 0
void reset_tank()
{
    check_all_valves_closed(); // make sure all outer valves are closed
    while (prop_system.get_pressure() > prop_system.pressure_delta) // arbitrary delta
        thrust();
    TEST_ASSERT(prop_system.get_pressure() < prop_system.pressure_delta);
}

void thrust()
{
    TEST_ASSERT_FALSE(prop_system.is_enabled);
    static const std::array<unsigned int, 4> schedule = {4, 2, 3, 6};
    prop_system.set_thrust_valve_schedule(schedule);
    prop_system.enable();
    delay(120); // outer valves have 120 seconds to finish thrust
    TEST_ASSERT_FALSE(prop_system.is_enabled);
    prop_system.disable();
}

// Test that opening inner valve increases the pressure
// Test that opening outer valves decrease pressure
void test_get_pressure()
{
    auto old_pressure = prop_system.get_pressure();
    prop_system.set_tank_valve_state(0, 1);
    delay(1000); // 1 seconds
    prop_system.set_tank_valve_state(0, 0);
    ASSERT_TRUE(prop_system.get_pressure() > old_pressure, 
    "new pressure of tank 2 should be higher after filling");

    old_pressure = prop_system.get_pressure();
    reset_tank();
    ASSERT_TRUE(prop_system.get_pressure() < old_pressure,
    "new pressure of tank 2 should be lower after firing");
}

// Test that the backup valve can fill the tank to threshold value
void test_backup_valve()
{
    auto old_pressure = prop_system.get_pressure();
    prop_system.set_tank_valve_state(1, 1);
    delay(1000);
    prop_system.set_tank_valve_state(1, 0);
    ASSERT_TRUE(prop_system.get_pressure() > old_pressure, 
    "new pressure of tank 2 should be lower after firing");
    reset_tank();
}

void test_get_temp_inner()
{
    // temperature increases with pressure
    reset_tank();
    auto old_temp = prop_system.get_temp_inner();
    
    fill_tank();
    ASSERT_TRUE( prop_system.get_temp_inner() < old_temp, 
    "new temp of tank 1 should be lower after filling");

    old_temp = prop_system.get_temp_inner();
    reset_tank();
    ASSERT_TRUE( prop_system.get_temp_inner() > old_temp, 
    "new temp of tank 1 should be the same after firing");
}

void test_get_temp_outer()
{
    reset_tank();
    auto old_temp = prop_system.get_temp_outer();
    fill_tank();
    auto new_temp = prop_system.get_temp_outer();
    ASSERT_TRUE(new_temp > old_temp, "new temp of tank 2 should be higher after filling");
    thrust();
}

void setup() {
    delay(5000);
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    prop_system.setup();
    UNITY_BEGIN();
    RUN_TEST(test_initialization);
    RUN_TEST(test_is_functional);
    RUN_TEST(test_set_tank_valve_state);
    RUN_TEST(test_set_thrust_valve_schedule);
    RUN_TEST(test_disable);
    RUN_TEST(test_enable);
    RUN_TEST(test_get_pressure);
    RUN_TEST(test_backup_valve);
    RUN_TEST(test_get_temp_inner);
    RUN_TEST(test_get_temp_outer);
    UNITY_END();
}
