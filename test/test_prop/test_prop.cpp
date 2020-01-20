#include <unity.h>
#define PROP_TEST
#include <PropulsionSystem.hpp>
#include <Arduino.h>

/**
 * test_prop.cpp
 * Hardware test for the Propulsion System
 * - Tests are split into Teensy only tests and the With Sensors tests
 */ 
#define ASSERT_TRUE(x, msg){UNITY_TEST_ASSERT(x, __LINE__, msg);}
#define ASSERT_FALSE(x, msg){UNITY_TEST_ASSERT(x, __LINE__, msg);}

/**
 * Helper Methods
 */
void check_all_valves_closed();
void check_tank2_schedule(const std::array<unsigned int, 4> &expected_schedule);
void reset_tank();
void fill_tank(); 
void thrust();
bool is_at_threshold_pressure();
void check_tank2_valve_status(bool a, bool b, bool c, bool d);
void check_tank1_valve_status(bool a, bool b);

const std::array<unsigned int, 4> zero_schedule = {0, 0, 0, 0};

Devices::PropulsionSystem prop_system;

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

// Make sure that the current schedule matches the provided schedule
void check_tank2_schedule(const std::array<unsigned int, 4> &expected_schedule)
{
    for (size_t i = 0; i < 4; ++i)
        TEST_ASSERT_EQUAL(expected_schedule[i], prop_system.tank2.schedule[i]);
}

// Make sure all pins low
void check_all_valves_closed()
{
    for (size_t i = 0; i < 2; ++i)
        TEST_ASSERT_EQUAL(LOW, prop_system.tank1.is_valve_open(i));
    for (size_t i = 0; i < 4; ++i)
        TEST_ASSERT_EQUAL(LOW, prop_system.tank2.is_valve_open(i));
}

void check_tank2_valve_status(bool a, bool b, bool c, bool d)
{
    TEST_ASSERT_EQUAL_MESSAGE(a, prop_system.tank2.is_valve_open(0), "tank2 valve 1");
    TEST_ASSERT_EQUAL_MESSAGE(b, prop_system.tank2.is_valve_open(1), "tank2 valve 2");
    TEST_ASSERT_EQUAL_MESSAGE(c, prop_system.tank2.is_valve_open(2), "tank2 valve 3");
    TEST_ASSERT_EQUAL_MESSAGE(d, prop_system.tank2.is_valve_open(3), "tank2 valve 4");
}

void check_tank1_valve_status(bool a, bool b)
{
    TEST_ASSERT_EQUAL_MESSAGE(a, prop_system.tank1.is_valve_open(0), "tank1 valve 1");
    TEST_ASSERT_EQUAL_MESSAGE(b, prop_system.tank1.is_valve_open(1), "tank1 valve 2");
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
    prop_system.set_schedule(0, 0, 0, 0, 0);
    prop_system.enable();
    ASSERT_FALSE(prop_system.is_tank2_ready(), "interval timer should not be on bc bad start time");
    prop_system.set_schedule(0, 0, 0, 0,micros() + 1000000 );
    ASSERT_TRUE(prop_system.is_tank2_ready(), "interval timer should be on");
    prop_system.enable();
    TEST_ASSERT_TRUE(prop_system.is_functional());
    check_all_valves_closed();
    // enable should not open any valves since schedule is 0
    ASSERT_FALSE(prop_system.tank2.is_lock_free(), 
    "thrust valves should be locked");
    delay(6); // make sure the interrupt didn't turn on the valves
    check_all_valves_closed();
}

// should disable interval and turn off all thrust valves
void test_disable()
{
    prop_system.disable();
    // TODO: how to check that intervaltimer is off
    ASSERT_FALSE(prop_system.is_tank2_ready(), "interval timer should be off");
    TEST_ASSERT_TRUE(prop_system.is_functional());
    // check that schedule is 0 and valves are all off
    check_tank2_schedule(zero_schedule);
    check_all_valves_closed();
}

void test_reset()
{
    prop_system.set_schedule(12, 1000, 40, 200, micros() + 1);
    // possibly dangerous
    prop_system.enable();
    delay(4); // by now, valve 0 should be opened
    check_tank2_valve_status(1, 0, 0, 0);
    delay(20);
    check_tank2_valve_status(0, 1, 1, 1);
    // reset stops the timer
    prop_system.reset();
    check_all_valves_closed();
    check_tank2_schedule(zero_schedule);
}

// test that we can open and close the tank valves
void test_open_tank1_valve()
{
    prop_system.open_valve(prop_system.tank1, 0);
    delay(100);
    ASSERT_TRUE(prop_system.tank1.is_valve_open(0), "tank1 valve 0 should be open");
    delay(1000); // fire for 1 second
    prop_system.close_valve(prop_system.tank1, 0);
    ASSERT_FALSE(prop_system.tank1.is_valve_open(0), "tank1 valve 0 should be closed"); // make sure it is closed
}

void test_tank_lock()
{
    prop_system.open_valve(prop_system.tank1, 1);
    ASSERT_FALSE(prop_system.tank2.is_lock_free(), "tank valves should be locked");
    delay(1000);
    prop_system.close_valve(prop_system.tank1, 1);
    ASSERT_FALSE (prop_system.tank2.is_lock_free(), "tank should still be locked");
    delay(1000* 5);
    ASSERT_FALSE (prop_system.tank2.is_lock_free(), "tank should still be locked");
    delay(1000*5); 
    ASSERT_TRUE (prop_system.tank2.is_lock_free(), "tank should be unlocked");
}

void test_tank1_enforce_lock()
{
    delay(1000);
    prop_system.open_valve(prop_system.tank1, 1);
    prop_system.close_valve(prop_system.tank1, 1);
    delay(3);
    while(!prop_system.open_valve(prop_system.tank1, 2)){}
    // ASSERT_FALSE(prop_system.open_valve(prop_system.tank1, 1), "Request to open tank1 valve 1 should be denied");
    // prop_system.close_valve(prop_system.tank1, 0);
    // ASSERT_FALSE(prop_system.open_valve(prop_system.tank1, 1), "Closing valve does not reset valve timer");
    // delay(1000*10);
    // ASSERT_TRUE(prop_system.open_valve(prop_system.tank1, 1), "Valve lock should be unlocked now");
    // delay(1000);
    // prop_system.close_valve(prop_system.tank1, 1);
}

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

// Testing tank2 schedule
void test_tank2_firing_schedule()
{
    TEST_ASSERT_FALSE(prop_system.is_tank2_ready());

    // static const std::array<unsigned int, 4> schedule = {30, 40, 50, 60};
    TEST_ASSERT_TRUE(prop_system.set_schedule(30, 40, 50, 60, micros() + 1*1000000));
    // fire when its 3 seconds into the future
    prop_system.enable();
    TEST_ASSERT_TRUE(prop_system.is_tank2_ready());
    // delayMicroseconds(1000000);
    // check_tank2_valve_status(0, 0, 0, 0);
    // delayMicroseconds(1000000);
    // check_tank2_valve_status(0, 0, 0, 0);
    // delayMicroseconds(997000);
    // check_tank2_valve_status(0, 0, 0, 0);
    // delayMicroseconds(3000);
    // check_tank2_valve_status(1, 0, 0, 0);
    // delayMicroseconds(3000);
    // check_tank2_valve_status(1, 1, 0, 0);
    // delayMicroseconds(3000);
    // check_tank2_valve_status(1, 1, 1, 0);
    // delayMicroseconds(3000);
    // check_tank2_valve_status(1, 1, 1, 1);
    // delayMicroseconds(200000);
    // check_tank2_valve_status(0, 1, 1, 1);
    // delayMicroseconds(100000);
    // check_tank2_valve_status(0, 0, 1, 1);
    // delayMicroseconds(100000);
    // check_tank2_valve_status(0, 0, 0, 1);
    // delayMicroseconds(500000);
    // check_all_valves_closed();
}

void test_tank2_start_time()
{
    prop_system.reset();
    // TEST_ASSERT_FALSE(prop_system.is_tank2_ready());
    // static const std::array<unsigned int, 4> schedule = {30, 40, 50, 60};
    prop_system.open_valve(prop_system.tank2, 3);
    // delay(500);
    // if (prop_system.tank2.is_lock_free())
    //     prop_system.open_valve(prop_system.tank2, 3);
    TEST_ASSERT_TRUE(prop_system.set_schedule(300 ,400, 500, 0, micros() + 500000))
    while(!prop_system.enable()) {}
    TEST_ASSERT_TRUE(prop_system.is_tank2_ready());
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

bool is_at_threshold_pressure()
{
    return prop_system.tank2.get_pressure() > prop_system.tank2.threshold_pressure;
}

// Empty out the outer tank so that pressure is 0
void reset_tank()
{
    // this is bad
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

void setup() {
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    while (!Serial)
        ;
    prop_system.setup();
    UNITY_BEGIN();
    // RUN_TEST(test_initialization);
    // RUN_TEST(test_is_functional);
    // RUN_TEST(test_open_tank1_valve);
    // RUN_TEST(test_tank_lock);
    // RUN_TEST(test_tank1_enforce_lock);
    // RUN_TEST(test_tank2_firing_schedule);
    RUN_TEST(test_tank2_start_time);
    // RUN_TEST(test_enable);
    // RUN_TEST(test_disable);
    // RUN_TEST(test_reset);
    // RUN_TEST(test_get_pressure);
    // RUN_TEST(test_backup_valve);
    // RUN_TEST(test_get_temp_inner);
    // RUN_TEST(test_get_temp_outer);
    UNITY_END();
}

void loop()
{

}