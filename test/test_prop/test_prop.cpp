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

void are_all_valves_closed();
void is_schedule_expected(const std::array<unsigned int, 4> &expected_schedule);
void reset_tank();
void fill_tank(); 
void thrust();

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
    are_all_valves_closed();
    // there should be no interrupts at startup
    TEST_ASSERT_FALSE(prop_system.is_enabled);
    // prop system should still be considered functional
    TEST_ASSERT_TRUE(prop_system.is_functional());
}

void are_all_valves_closed()
{
    for (size_t i = 0; i < 4; ++i)
        TEST_ASSERT_EQUAL(0, prop_system.is_valve_opened[i]); 
}

void is_schedule_expected(const std::array<unsigned int, 4> &expected_schedule)
{
    for (size_t i = 0; i < 4; ++i)
        TEST_ASSERT_EQUAL(expected_schedule[i], prop_system.thrust_valve_schedule);
}

void test_is_functional()
{
    TEST_ASSERT_TRUE(prop_system.is_functional());
}

void test_enable()
{
    TEST_ASSERT_TRUE(prop_system.is_functional());
    prop_system.enable();
    TEST_ASSERT_TRUE(prop_system.is_enabled);
    TEST_ASSERT_TRUE(prop_system.is_functional());
}

void test_disable()
{
    prop_system.disable();
    TEST_ASSERT_FALSE(prop_system.is_enabled);
    TEST_ASSERT_TRUE(prop_system.is_functional());
    // check that schedule is 0 and valves are all off
    is_schedule_expected(zero_schedule);
    are_all_valves_closed();
}

void test_set_tank_valve_state()
{
    TEST_ASSERT_FALSE(prop_system.is_enabled);
    prop_system.set_tank_valve_state(0, 1);
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
    
}

// Fill tank to threshold value
void fill_tank()
{
    prop_system.set_tank_valve_state(0, 1);
    // Keep valve open until we are within delta of threshold pressure
    while ( abs(prop_system.get_pressure() - prop_system.threshold_pressure) > prop_system.pressure_delta)
        delay(1);
    prop_system.set_tank_valve_state(0, 0);
}

// Empty out the outer tank so that pressure is 0
void reset_tank()
{
    are_all_valves_closed(); // make sure all outer valves are closed
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
    TEST_ASSERT_TRUE(prop_system.get_pressure() > old_pressure);

    old_pressure = prop_system.get_pressure();
    reset_tank();
    TEST_ASSERT_TRUE(prop_system.get_pressure() < old_pressure);
}

// Test that the backup valve can fill the tank to threshold value
void test_backup_valve()
{
    auto old_pressure = prop_system.get_pressure();
    prop_system.set_tank_valve_state(1, 1);
    delay(1000);
    prop_system.set_tank_valve_state(1, 0);
    TEST_ASSERT_TRUE(prop_system.get_pressure() > old_pressure);
    reset_tank();
}

void test_get_temp_inner()
{
    // temperature increases with pressure
    reset_tank();
    auto old_temp = prop_system.get_temp_inner();
    fill_tank();
    auto new_temp = prop_system.get_temp_inner();
    TEST_ASSERT_TRUE(new_temp < old_temp);
}

void test_get_temp_outer()
{
    reset_tank();
    auto old_temp = prop_system.get_temp_outer();
    fill_tank();
    auto new_temp = prop_system.get_temp_outer();
    TEST_ASSERT_TRUE(new_temp > old_temp);
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
