
#include "prop_common.h"

/**
 * test_prop.cpp
 * Hardware test for the Propulsion System
 * - Tests are split into Teensy only tests and the With Sensors tests
 * These are the Teensy only tests
 */ 

#define TO_MICRO(x) x*1000
using namespace Devices;

PropulsionSystem prop_system;

/* TimedLock Tests */

void test_timedlock_init()
{
    TimedLock tl;
    ASSERT_TRUE(tl.is_free(), "tl should be free at initialization");
}

void test_timedlock_procure()
{
    TimedLock tl;
    ASSERT_TRUE(tl.procure(TO_MICRO(500)), "Lock tl for 500 ms");
    ASSERT_FALSE(tl.is_free(), "Should not be free"); 
    ASSERT_FALSE(tl.procure(TO_MICRO(1000)), "Procuring tl should fail"); 
    delay(510);
    ASSERT_TRUE(tl.is_free(), "Should be free now");
    ASSERT_TRUE(tl.procure(TO_MICRO(1000)), "Should be able to procure now");
    TEST_ASSERT_FALSE(tl.is_free());
}

void test_safe_add()
{
    uint32_t a = UINT32_MAX -1;
    uint32_t b = 2; 
    TEST_ASSERT_EQUAL(0, TimedLock::safe_add(a, b));
    TEST_ASSERT_EQUAL(UINT32_MAX, TimedLock::safe_add(a, 1));
    TEST_ASSERT_EQUAL(0, TimedLock::safe_add(0, 0));
}

void test_safe_subtract()
{
    TEST_ASSERT_EQUAL(0, TimedLock::safe_subtract(1, 2));
    TEST_ASSERT_EQUAL(0, TimedLock::safe_subtract(UINT32_MAX, UINT32_MAX));
    TEST_ASSERT_EQUAL(1, TimedLock::safe_subtract(2, 1));
}

void timed_lock_tests()
{
    RUN_TEST(test_timedlock_init);
    RUN_TEST(test_timedlock_procure);
    RUN_TEST(test_safe_add);
    RUN_TEST(test_safe_subtract);
}

/* PropulsionSystem Tests */

void test_initialization()
{
    // schedule should be initialized to 0
    check_tank2_schedule(zero_schedule);
    // all valves should be closed
    check_all_valves_closed();
    // there should be no interrupts at startup
    TEST_ASSERT_FALSE(prop_system.is_tank2_ready());
}

void test_is_start_time_ok()
{
    ASSERT_FALSE(prop_system.is_start_time_ok(micros() + 3000), "cannot request to start within 3 ms of current time");
    uint32_t start_time = TimedLock::safe_add(micros(), 3010);
    ASSERT_TRUE(prop_system.is_start_time_ok(start_time), "ok to start 3.01 ms from current time");
}

/* Tank2 scheduling Tests */

void test_set_schedule_bad_start_time()
{
    ASSERT_FALSE(prop_system.is_tank2_ready(), "sanity check");
    ASSERT_FALSE(prop_system.set_schedule(2, 3, 4, 5, micros() + 3000), "should fail if start_time <= now + 3ms");
}

void test_set_schedule_bad_valve_times()
{
    ASSERT_FALSE(prop_system.is_tank2_ready(), "sanity");
    ASSERT_FALSE(prop_system.set_schedule(2, 3, 1000, 5, micros() + 5000), "should fail to set 1 second schedule");
}

void test_set_schedule_good()
{
    ASSERT_FALSE(prop_system.is_tank2_ready(), "sanity");
    ASSERT_TRUE(prop_system.set_schedule(2, 3, 999, 5, micros() + 3010), "this should work");
    ASSERT_FALSE(prop_system.is_tank2_ready(), "should be false because didnt call enable");
}

void scheduling_tests()
{
    RUN_TEST(test_set_schedule_bad_start_time);
    RUN_TEST(test_set_schedule_bad_valve_times);
    RUN_TEST(test_set_schedule_good);
}

/* Tank2 enable tests */

void test_enable_bad_start_time()
{
    ASSERT_FALSE(prop_system.is_tank2_ready(), "sanity check");
    TEST_ASSERT_TRUE(prop_system.set_schedule(2, 3, 999, 5, micros() + 3010));
    delay(500);
    ASSERT_FALSE(prop_system.enable(), "enable should fail because we cannot start on time");
    check_all_valves_closed();
}

void test_enable_success()
{
    ASSERT_FALSE(prop_system.is_tank2_ready(), "sanity check");  
    TEST_ASSERT_TRUE(prop_system.set_schedule(2, 3, 999, 5, micros() + 5000));

    ASSERT_TRUE(prop_system.enable(), "enable should pass because we can do this on time");
    ASSERT_TRUE(prop_system.is_tank2_ready(), "tank2 should be ready to fire now");
    prop_system.disable();
}

void test_disable()
{
    TEST_ASSERT_TRUE(prop_system.set_schedule(2, 3, 999, 5, micros() + 5000));
    TEST_ASSERT_TRUE(prop_system.enable());
    ASSERT_TRUE(prop_system.is_tank2_ready(), "tank2 should be ready since we just enabled it");

    TEST_ASSERT_TRUE(prop_system.is_tank2_ready());
    prop_system.disable();
    TEST_ASSERT_FALSE(prop_system.is_tank2_ready());
}

void test_reenable()
{
    prop_system.reset();
    TEST_ASSERT_TRUE(prop_system.set_schedule(2, 3, 999, 5, micros() + 5000));
    TEST_ASSERT_TRUE(prop_system.enable());
    TEST_ASSERT_TRUE(prop_system.is_tank2_ready());

    prop_system.disable();
    TEST_ASSERT_FALSE(prop_system.is_tank2_ready());
    ASSERT_TRUE(prop_system.enable(), "test that tank2 lock unlocks prematurely if we have not fired");
    ASSERT_TRUE(prop_system.is_tank2_ready(), "should be allowed to enable it since we can still fire in time");
    TEST_ASSERT_TRUE(prop_system.is_tank2_ready());

    prop_system.disable();
    prop_system.clear_schedule();
}

void test_disable_while_firing()
{
    TEST_ASSERT_TRUE(prop_system.set_schedule(42, 42, 42, 42, micros() + 3100));
    TEST_ASSERT_TRUE(prop_system.enable());
    delay(1);
    TEST_ASSERT_FALSE(prop_system.is_done_firing());
    prop_system.disable();
    TEST_ASSERT_FALSE(prop_system.is_tank2_ready());
}

void test_cannot_clear_schedule_when_enabled()
{
    TEST_ASSERT_TRUE(prop_system.set_schedule(42, 42, 42, 42, micros() + 6969));
    TEST_ASSERT_TRUE(prop_system.enable());
    TEST_ASSERT_FALSE(prop_system.clear_schedule());
    prop_system.disable();
    TEST_ASSERT_TRUE(prop_system.clear_schedule());
}

void interval_timer_tests()
{
    RUN_TEST(test_enable_bad_start_time);
    RUN_TEST(test_enable_success);
    RUN_TEST(test_disable);
    RUN_TEST(test_reenable);
    RUN_TEST(test_disable_while_firing);
    RUN_TEST(test_cannot_clear_schedule_when_enabled);
}

void test_open_tank1_valve()
{
    prop_system.reset();
    prop_system.open_valve(prop_system.tank1, 0);
    delay(100);
    ASSERT_TRUE(prop_system.tank1.is_valve_open(0), "tank1 valve 0 should be open");
    delay(1000); // fire for 1 second
    prop_system.close_valve(prop_system.tank1, 0);
    ASSERT_FALSE(prop_system.tank1.is_valve_open(0), "tank1 valve 0 should be closed"); // make sure it is closed
}

void test_open_both_valves()
{
    prop_system.reset();
    TEST_ASSERT_TRUE(prop_system.set_schedule(12, 999, 40, 200, micros() + 3010));
    TEST_ASSERT_TRUE(prop_system.enable());
    TEST_ASSERT_FALSE(prop_system.is_done_firing())
    while (!prop_system.tank2.is_valve_open(0)){}
    check_tank2_valve_status(1, 0, 0, 0);
    ASSERT_TRUE(prop_system.open_valve(prop_system.tank1, 0), "We should be able to open tank1 valves when firing tank2, without messing up the schedule");
    delayMicroseconds(TO_MICRO(3)); // Checking that opening valve from tank1 does not mess up schedule
    check_tank2_valve_status(1, 1, 0, 0);
    delayMicroseconds(TO_MICRO(6));
    check_tank2_valve_status(1, 1, 1, 1);
    prop_system.close_valve(prop_system.tank1, 0);
    delayMicroseconds(TO_MICRO(3));
    check_tank2_valve_status(0, 1, 1, 1);
    prop_system.reset();
    check_all_valves_closed();
}
void test_ignore_short_schedules()
{
    prop_system.reset();
    TEST_ASSERT_TRUE(prop_system.set_schedule(12, 9, 40, 200, micros() + 3250));
    TEST_ASSERT_FALSE(prop_system.is_done_firing());
    TEST_ASSERT_TRUE(prop_system.enable());
    while (!prop_system.tank2.is_valve_open(0)){}
    check_tank2_valve_status(1, 0, 0, 0);  
    delayMicroseconds(TO_MICRO(3)); 
    // valve 2 is flipped one cycle early since we don't flip valve 1
    check_tank2_valve_status(1, 0, 1, 0);
    delayMicroseconds(TO_MICRO(3));
    check_tank2_valve_status(1, 0, 1, 1);
    prop_system.disable();
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
    TEST_ASSERT_FALSE(prop_system.is_tank2_ready());
    ASSERT_TRUE(
        prop_system.open_valve(prop_system.tank1, 1),
        "First, open valve tank 1 valve to trigger 10 s mandatory wait");
    TEST_ASSERT_TRUE(TimedLock::safe_add(micros(), TO_MICRO(3069)));
    ASSERT_TRUE(
        prop_system.set_schedule(999, 2, 400, 100, micros() + TO_MICRO(3069)),
        "Set the tank2 timer and schedule long firings");
    TEST_ASSERT_TRUE(prop_system.enable());
    // Wait for tank2 schedule to start
    while (!prop_system.tank2.is_valve_open(0)){}
    ASSERT_FALSE(prop_system.is_done_firing(), "tank2 is definitely firing now");
    prop_system.reset();
    ASSERT_FALSE(prop_system.is_done_firing(), "false since we didint finish firing");
    check_all_valves_closed();
    ASSERT_FALSE(prop_system.is_tank2_ready(), "tank2 timer should not be on");
    check_tank2_schedule(zero_schedule);
    ASSERT_TRUE(prop_system.tank1.is_lock_free(), "tank1 lock should have been reset");
    ASSERT_TRUE(prop_system.tank2.is_lock_free(), "tank2 lock should have been reset");
}

void test_tank_lock()
{
    prop_system.reset();
    ASSERT_TRUE(prop_system.open_valve(prop_system.tank1, 1), "Open tank1 valve");
    ASSERT_FALSE(prop_system.tank1.is_lock_free(), "tank valves should be locked");
    delay(1000);
    prop_system.close_valve(prop_system.tank1, 1);
    ASSERT_FALSE (prop_system.tank1.is_lock_free(), "tank should still be locked");
    delay(1000* 5);
    ASSERT_FALSE (prop_system.tank1.is_lock_free(), "tank should still be locked");
    delay(1000*5); 
    ASSERT_TRUE (prop_system.tank1.is_lock_free(), "tank should be unlocked");
}

void test_tank1_enforce_lock()
{
    ASSERT_TRUE(prop_system.open_valve(prop_system.tank1, 1), "open tank1 valve 1");
    ASSERT_TRUE(prop_system.tank1.is_valve_open(1), "valve 1 should be opened");
    ASSERT_FALSE(prop_system.tank1.is_lock_free(), "lock should not be free");
    delay(3);
    ASSERT_FALSE(prop_system.open_valve(prop_system.tank1, 0), "Request to open tank1 valve 0 should be denied");
    prop_system.close_valve(prop_system.tank1, 0);
    ASSERT_FALSE(prop_system.open_valve(prop_system.tank1, 1), "Closing valve 1 does not reset valve timer");
    delayMicroseconds( TO_MICRO(1000*10));
    ASSERT_TRUE(prop_system.open_valve(prop_system.tank1, 1), "Valve lock should be unlocked now");
    prop_system.reset();
}

// Testing tank2 schedule
// Verified with oscilliscope
void test_tank2_firing_schedule()
{
    prop_system.reset();
    TEST_ASSERT_FALSE(prop_system.is_tank2_ready());

    TEST_ASSERT_TRUE(
        prop_system.set_schedule(300, 400, 500, 600, micros() + TO_MICRO(12))
    );
    // fire when its 3 seconds into the future
    prop_system.enable();
    TEST_ASSERT_TRUE(prop_system.is_tank2_ready());
    // Not sure how fast instructions run, so just wait for the first one
    while (!prop_system.tank2.is_valve_open(0)){}
    TEST_ASSERT_FALSE(prop_system.is_done_firing());
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
    TEST_ASSERT_TRUE(prop_system.is_done_firing());
}

void test_tank2_start_time()
{
    prop_system.reset();
    TEST_ASSERT_FALSE(prop_system.is_tank2_ready());
    prop_system.open_valve(prop_system.tank2, 3);
    delay(500);
    if (prop_system.tank2.is_lock_free())
        prop_system.open_valve(prop_system.tank2, 3);
    TEST_ASSERT_TRUE(prop_system.set_schedule(300 ,400, 500, 0, micros() + 500000))
    while(!prop_system.enable()) {}
    TEST_ASSERT_TRUE(prop_system.is_tank2_ready());
    prop_system.reset();
}

void setup() {
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    while (!Serial)
        ;
    prop_system.setup();
    UNITY_BEGIN();
    timed_lock_tests();
    RUN_TEST(test_initialization);
    RUN_TEST(test_is_start_time_ok);
    scheduling_tests();
    RUN_TEST(test_reset);
    interval_timer_tests();
    open_valve_tests();
    RUN_TEST(test_open_tank1_valve);
    RUN_TEST(test_tank_lock);
    RUN_TEST(test_tank1_enforce_lock);
    RUN_TEST(test_tank2_firing_schedule);
    RUN_TEST(test_tank2_start_time);

    // test that prop disables itself when its done with the schedule
    UNITY_END();
}

void loop()
{

}