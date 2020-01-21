#include "../test_prop/prop_common.h"
using namespace Devices;
/**
 * test_prop_with_sensors.cpp
 * Hardware test for the Propulsion System
 * - Tests are split into Teensy only tests and the With Sensors tests
 * This is teensy_with_sensors
 */ 
#define ASSERT_TRUE(x, msg){UNITY_TEST_ASSERT(x, __LINE__, msg);}
#define ASSERT_FALSE(x, msg){UNITY_TEST_ASSERT(x, __LINE__, msg);}

void thrust();
bool is_at_threshold_pressure();
void reset_tank();
void fill_tank(); 

#define THRESHOLD_PRESSURE 25

Devices::PropulsionSystem prop_system;
// Open tank valve for 1 second and wait 10 s to make sure no one can fire

void fire_tank_valve(size_t valve_pin)
{
    if (valve_pin > 2)
    {
        ASSERT_FALSE(1, "Tank Valve pins are indexed at 0 and 1");
    }
    while ( !prop_system.open_valve(prop_system.tank1, 1) ) 
        delay(140);

    delay(1000);
    prop_system.close_valve(prop_system.tank1, 1);
}

void test_is_functional()
{
    TEST_ASSERT_TRUE(prop_system.is_functional());
}

void test_initialization()
{
    // schedule should be initialized to 0
    check_tank2_schedule(zero_schedule);
    // all valves should be closed
    check_all_valves_closed();
    // there should be no interrupts at startup
    TEST_ASSERT_FALSE(prop_system.is_tank2_ready());
    // prop system should still be considered functional
    TEST_ASSERT_TRUE(prop_system.is_functional());
}

// Fill tank to threshold value
void fill_tank()
{
    // Fire a maximum of 20 times for 1 s every 10s until we have threshold_presure
    for ( size_t i = 0; is_at_threshold_pressure() && i < 20; ++i)
    {
        prop_system.open_valve(prop_system.tank1, 0);
        delay(1000);
        prop_system.close_valve(prop_system.tank1, 0);
        delay(1000* 10); // fire every 10 seconds
    }
    ASSERT_TRUE(is_at_threshold_pressure(), "tank 2 pressure should be above threshold pressure")
}


// Empty out the outer tank so that pressure is 0
void reset_tank()
{
    check_all_valves_closed(); // make sure all outer valves are closed
    while (prop_system.tank2.get_pressure() > 1.0f) // arbitrary delta
        thrust();
    // ASSERT_TRUE(prop_system.get_pressure() < prop_system.pressure_delta, 
    // "tank 2 pressure should be empty");
}

void thrust()
{
    ASSERT_FALSE(prop_system.is_tank2_ready(), 
    "interval timer should not be on when we have not yet started thrust");
    // static const std::array<unsigned int, 4> schedule = {4, 2, 3, 6};
    ASSERT_FALSE(prop_system.tank1.is_valve_open(0) || prop_system.tank1.is_valve_open(1), 
    "Both tank valves should be closed");
    prop_system.set_schedule(4, 2, 3, 6, micros() + 1);
    prop_system.enable();
    delay(120); // outer valves have 120 seconds to finish thrust
    TEST_ASSERT_FALSE(prop_system.is_tank2_ready());
    prop_system.disable();
}


bool is_at_threshold_pressure()
{
    return prop_system.tank2.get_pressure() > THRESHOLD_PRESSURE;
}

void test_get_temp_inner()
{
    // temperature increases with pressure
    reset_tank();
    auto old_temp = prop_system.tank1.get_temp();

    fill_tank();
    ASSERT_TRUE( prop_system.tank1.get_temp() < old_temp, 
    "temp of tank 1 should be lower after filling");
    old_temp = prop_system.tank1.get_temp();

    reset_tank();
    ASSERT_TRUE( prop_system.tank1.get_temp() > old_temp, 
    "temp of tank 1 should be the same after firing");
}

void test_get_temp_outer()
{
    reset_tank();
    auto old_temp = prop_system.tank2.get_temp();
    fill_tank();
    auto new_temp = prop_system.tank2.get_temp();
    ASSERT_TRUE(new_temp > old_temp, 
    "temp of tank 2 should be higher after filling");
    old_temp = new_temp;
    thrust();
    ASSERT_TRUE(prop_system.tank2.get_temp() < old_temp, 
    "temp of tank 2 should be lower after firing");
}

// Test that opening inner valve increases the pressure
// Test that opening outer valves decrease pressure
void test_get_pressure()
{
    auto old_pressure = prop_system.tank2.get_pressure();
    prop_system.open_valve(prop_system.tank1, 0);
    delay(1000); // 1 seconds
    prop_system.close_valve(prop_system.tank1, 0);
    ASSERT_TRUE(prop_system.tank2.get_pressure() > old_pressure, 
    "pressure of tank 2 should be higher after filling");

    old_pressure = prop_system.tank2.get_pressure();
    reset_tank();
    ASSERT_TRUE(prop_system.tank2.get_pressure() < old_pressure,
    "pressure of tank 2 should be lower after firing");
}

// Test that the backup valve can fill the tank to threshold value
void test_backup_valve()
{
    auto old_pressure = prop_system.tank2.get_pressure();
    prop_system.open_valve(prop_system.tank1, 1);
    delay(1000);
    ASSERT_TRUE(prop_system.tank1.is_valve_open(1), "tank 1 valve 1 should be opened");
    prop_system.close_valve(prop_system.tank1, 1);
    ASSERT_TRUE(prop_system.tank2.get_pressure() > old_pressure, 
    "pressure of tank 2 should be higher after filling");
    reset_tank();
    ASSERT_TRUE(prop_system.tank2.get_pressure() < old_pressure,
    "pressure of tank 2 should be lower after firing");
}

void setup() {
     Serial.begin(9600);
    pinMode(13, OUTPUT);
    while (!Serial)
        ;
    prop_system.setup();
    UNITY_BEGIN();
    RUN_TEST(test_initialization);
    RUN_TEST(test_is_functional);
    RUN_TEST(test_get_pressure);
    RUN_TEST(test_backup_valve);
    RUN_TEST(test_get_temp_inner);
    RUN_TEST(test_get_temp_outer);
    UNITY_END();
}

void loop()
{

}