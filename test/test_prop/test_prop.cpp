
#include "prop_common.h"

/**
 * test_prop.cpp
 * Hardware test for the Propulsion System
 * - Tests are split into Teensy only tests and the With Sensors tests
 * These are the Teensy only tests
 */ 

using namespace Devices;

#define TO_MICRO(x) x*1000


/* PropulsionSystem Tests */

void test_initialization()
{
    // schedule should be initialized to 0
    check_tank2_schedule(zero_schedule);
    // all valves should be closed
    check_all_valves_closed();
    // there should be no interrupts at startup
    TEST_ASSERT_FALSE(PropulsionSystem.is_firing());
}


/* Tank2 scheduling Tests */

void test_set_schedule_bad_valve_times()
{
    ASSERT_FALSE(PropulsionSystem.is_firing(), "sanity");
    ASSERT_FALSE(PropulsionSystem.set_schedule(2, 3, 1000, 5), "should fail to set 1 second schedule");
}

void test_set_schedule_good()
{
    ASSERT_FALSE(PropulsionSystem.is_firing(), "sanity");
    ASSERT_TRUE(PropulsionSystem.set_schedule(2, 3, 999, 5), "this should work");
    ASSERT_FALSE(PropulsionSystem.is_firing(), "should be false because didnt call enable");
}

void scheduling_tests()
{
    RUN_TEST(test_set_schedule_bad_valve_times);
    RUN_TEST(test_set_schedule_good);
}

/* Tank2 enable tests */
void test_enable_success()
{
    ASSERT_FALSE(PropulsionSystem.is_firing(), "sanity check");  
    TEST_ASSERT_TRUE(PropulsionSystem.set_schedule(2, 3, 999, 5));

    ASSERT_TRUE(PropulsionSystem.start_firing(), "enable should be ok");
    ASSERT_TRUE(PropulsionSystem.is_firing(), "tank2 should be firing right now");
    PropulsionSystem.disable();
}

void test_disable()
{
    TEST_ASSERT_TRUE(PropulsionSystem.set_schedule(2, 3, 999, 5));
    TEST_ASSERT_TRUE(PropulsionSystem.start_firing());
    ASSERT_TRUE(PropulsionSystem.is_firing(), "tank2 should be enabled");

    TEST_ASSERT_TRUE(PropulsionSystem.is_firing());
    PropulsionSystem.disable();
    TEST_ASSERT_FALSE(PropulsionSystem.is_firing());
}

void test_restart_firing()
{
    PropulsionSystem.reset();
    TEST_ASSERT_TRUE(PropulsionSystem.set_schedule(2, 3, 999, 5));
    TEST_ASSERT_TRUE(PropulsionSystem.start_firing());
    TEST_ASSERT_TRUE(PropulsionSystem.is_firing());

    PropulsionSystem.disable();
    TEST_ASSERT_FALSE(PropulsionSystem.is_firing());
    ASSERT_TRUE(PropulsionSystem.start_firing(), "test that we can restart the schedule");
    ASSERT_TRUE(PropulsionSystem.is_firing(), "should be allowed to enable it since there is still time");
    TEST_ASSERT_TRUE(PropulsionSystem.is_firing());

    PropulsionSystem.disable();
    PropulsionSystem.clear_schedule();
}

void test_disable_while_firing()
{
    TEST_ASSERT_TRUE(PropulsionSystem.set_schedule(42, 42, 42, 42));
    TEST_ASSERT_TRUE(PropulsionSystem.start_firing());
    delay(1);
    PropulsionSystem.disable();
    TEST_ASSERT_FALSE(PropulsionSystem.is_firing());
}

void test_cannot_clear_schedule_when_enabled()
{
    TEST_ASSERT_TRUE(PropulsionSystem.set_schedule(42, 42, 42, 42));
    TEST_ASSERT_TRUE(PropulsionSystem.start_firing());
    TEST_ASSERT_FALSE(PropulsionSystem.clear_schedule());
    PropulsionSystem.disable();
    TEST_ASSERT_TRUE(PropulsionSystem.clear_schedule());
}

void interval_timer_tests()
{
    RUN_TEST(test_enable_success);
    RUN_TEST(test_disable);
    RUN_TEST(test_restart_firing);
    RUN_TEST(test_disable_while_firing);
    RUN_TEST(test_cannot_clear_schedule_when_enabled);
}

void test_open_tank1_valve()
{
    PropulsionSystem.reset();
    PropulsionSystem.open_valve(Tank1, 0);
    delay(100);
    ASSERT_TRUE(Tank1.is_valve_open(0), "tank1 valve 0 should be open");
    delay(1000); // fire for 1 second
    PropulsionSystem.close_valve(Tank1, 0);
    ASSERT_FALSE(Tank1.is_valve_open(0), "tank1 valve 0 should be closed"); // make sure it is closed
}

void test_open_both_valves()
{
    PropulsionSystem.reset();
    TEST_ASSERT_TRUE(PropulsionSystem.set_schedule(12, 999, 40, 200));
    TEST_ASSERT_TRUE(PropulsionSystem.start_firing());
    while (!Tank2.is_valve_open(0)){}
    check_tank2_valve_status(1, 0, 0, 0);
    ASSERT_TRUE(PropulsionSystem.open_valve(Tank1, 0), "We should be able to open tank1 valves when firing tank2, without messing up the schedule");
    delayMicroseconds(TO_MICRO(3)); // Checking that opening valve from tank1 does not mess up schedule
    check_tank2_valve_status(1, 1, 0, 0);
    delayMicroseconds(TO_MICRO(6));
    check_tank2_valve_status(1, 1, 1, 1);
    PropulsionSystem.close_valve(Tank1, 0);
    delayMicroseconds(TO_MICRO(3));
    check_tank2_valve_status(0, 1, 1, 1);
    PropulsionSystem.reset();
    check_all_valves_closed();
}
void test_ignore_short_schedules()
{
    PropulsionSystem.reset();
    TEST_ASSERT_TRUE(PropulsionSystem.set_schedule(12, 9, 40, 200));
    TEST_ASSERT_TRUE(PropulsionSystem.start_firing());
    while (!Tank2.is_valve_open(0)){}
    check_tank2_valve_status(1, 0, 0, 0);  
    delayMicroseconds(TO_MICRO(3)); 
    // valve 2 is flipped one cycle early since we don't flip valve 1
    check_tank2_valve_status(1, 0, 1, 0);
    delayMicroseconds(TO_MICRO(3));
    check_tank2_valve_status(1, 0, 1, 1);
    PropulsionSystem.disable();
    check_all_valves_closed();
}

void open_valve_tests()
{
    RUN_TEST(test_open_tank1_valve);
    RUN_TEST(test_open_both_valves);
    RUN_TEST(test_ignore_short_schedules);
}

void test_reset()
{
    TEST_ASSERT_FALSE(PropulsionSystem.is_firing());
    ASSERT_TRUE(
        PropulsionSystem.open_valve(Tank1, 1),
        "open tank1 valve");
    ASSERT_TRUE(
        PropulsionSystem.set_schedule(999, 2, 400, 100),
        "Should be allowed to set tank2 schedule");
    TEST_ASSERT_TRUE(PropulsionSystem.start_firing());
    // Wait for tank2 schedule to start
    while (!Tank2.is_valve_open(0)){}
    PropulsionSystem.reset();
    check_all_valves_closed();
    ASSERT_FALSE(PropulsionSystem.is_firing(), "tank2 should not be firing");
    check_tank2_schedule(zero_schedule);
}


// Testing tank2 schedule
// Verified with oscilliscope
void test_tank2_firing_schedule()
{
    PropulsionSystem.reset();
    TEST_ASSERT_FALSE(PropulsionSystem.is_firing());

    TEST_ASSERT_TRUE(
        PropulsionSystem.set_schedule(300, 400, 500, 600)
    );
    // fire when its 3 seconds into the future
    PropulsionSystem.start_firing();
    TEST_ASSERT_TRUE(PropulsionSystem.is_firing());
    // Not sure how fast instructions run, so just wait for the first one
    while (!Tank2.is_valve_open(0)){}
    check_tank2_valve_status(1, 0, 0, 0);
    delayMicroseconds(TO_MICRO(3));
    check_tank2_valve_status(1, 1, 0, 0);
    delayMicroseconds(TO_MICRO(3));
    check_tank2_valve_status(1, 1, 1, 0);
    delayMicroseconds(TO_MICRO(3));
    check_tank2_valve_status(1, 1, 1, 1);
    delayMicroseconds(TO_MICRO(300));
    check_tank2_valve_status(0, 1, 1, 1);
    delayMicroseconds(TO_MICRO(100));
    check_tank2_valve_status(0, 0, 1, 1);
    delayMicroseconds(TO_MICRO(100));
    check_tank2_valve_status(0, 0, 0, 1);
    delayMicroseconds(TO_MICRO(100));
    check_all_valves_closed();
}


void setup() {
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    while (!Serial)
        ;
    PropulsionSystem.setup();
    UNITY_BEGIN();
    RUN_TEST(test_initialization);
    scheduling_tests();
    RUN_TEST(test_reset);
    interval_timer_tests();
    open_valve_tests();
    RUN_TEST(test_open_tank1_valve);
    RUN_TEST(test_tank2_firing_schedule);
    UNITY_END();
}

void loop()
{

}