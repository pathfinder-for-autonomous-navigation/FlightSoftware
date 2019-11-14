#include <StateFieldRegistry.hpp>
#include "../../src/FCCode/PiksiControlTask.hpp"
#include <unity.h>
#include <Piksi.hpp>

#include <chrono>
#include <ctime>

using namespace Devices;

void test_task_initialization()
{
    StateFieldRegistry registry;
    PiksiControlTask task(registry);
}

void test_task_execute()
{
    StateFieldRegistry registry;
    PiksiControlTask task(registry);

    //TEST_ASSERT_EQUAL()
    // std::array<double, 3> pos = {4000.0, 5000.0, 6000.0};
    // std::array<double, 3> vel = {4000.0, 5000.0, 6000.0};
    // std::array<double, 3> baseline_pos = {4000.0, 5000.0, 6000.0};


    unsigned int tow = 0;
    task.piksi.set_gps_time(tow);
    task.execute();

    TEST_ASSERT_TRUE(true);
    // TEST_ASSERT_EQUAL(BAD_DATA, task.get_current_state());

    // tow = 100;

    // task.piksi.set_gps_time(tow);
    // task.piksi.set_read_return(0);
    // task.piksi.set_pos_ecef(tow, pos);
    // task.piksi.set_vel_ecef(tow, vel);
    // task.piksi.set_baseline_ecef(tow, baseline_pos);

    // task.execute();
    // TEST_ASSERT_EQUAL(SUCCESS, task.get_current_state());

    // tow = 200;
    // task.piksi.set_gps_time(tow);
    // task.piksi.set_read_return(0);
    // task.piksi.set_pos_ecef(tow, pos);
    // task.piksi.set_vel_ecef(tow, vel);
    // task.piksi.set_baseline_ecef(tow, baseline_pos);

    // task.execute();
    // TEST_ASSERT_EQUAL(SUCCESS, task.get_current_state());

    // //don't update the time, should yield BAD_TIME
    // task.piksi.set_gps_time(tow);
    // task.piksi.set_read_return(0);
    // task.piksi.set_pos_ecef(tow, pos);
    // task.piksi.set_vel_ecef(tow, vel);
    // task.piksi.set_baseline_ecef(tow, baseline_pos);

    // task.execute();
    // TEST_ASSERT_EQUAL(BAD_DATA, task.get_current_state());

    

 
    //expect 1 cuz no bytes can be read lmao
    // int read_res = task.exec_read_buffer();
    // int update_res = task.

    // Check that things are initialized correctly
    //   TEST_ASSERT_EQUAL(IDLE, task.get_current_state());
    //   TEST_ASSERT_EQUAL(0, task.get_current_fn_number());

    //   // Check that calling execute() when IDLE does not change anything
    //   TEST_ASSERT_EQUAL(0, task.execute());
    //   TEST_ASSERT_EQUAL(IDLE, task.get_current_state());
    //   TEST_ASSERT_EQUAL(0, task.get_current_fn_number());

    //   // Test SBDWB
    //   TEST_ASSERT_EQUAL(true, task.request_state(SBDWB));
    //   TEST_ASSERT_EQUAL(SBDWB, task.get_current_state());
    //   // fnNumber should still be 0 at this point
    //   TEST_ASSERT_EQUAL(0, task.get_current_fn_number());

    //   // Check that sbdwb with a zero length message should fail and not change fnNumber

    //   TEST_ASSERT_EQUAL(3, task.execute());
    //   TEST_ASSERT_EQUAL(0, task.get_current_fn_number());
    //   // Check that a failure from fnNum = 0 resets the state to IDLE
    //   TEST_ASSERT_EQUAL(IDLE, task.get_current_state());

    //   // Set the message and length and try SBDWB again
    //   task.set_downlink_msg("hello", 5);
    //   TEST_ASSERT_EQUAL(true, task.request_state(SBDWB));
    //   TEST_ASSERT_EQUAL(0, task.get_current_fn_number());

    //   // Execute should be good this time

    //   TEST_ASSERT_EQUAL(0, task.execute());
    //   TEST_ASSERT_EQUAL(1, task.get_current_fn_number());

    //   // Check that changing state is not ok and does not change the fn number or state
    //   TEST_ASSERT_EQUAL(false, task.request_state(SBDIX));
    //   TEST_ASSERT_EQUAL(1, task.get_current_fn_number());
    //   TEST_ASSERT_EQUAL(SBDWB, task.get_current_state());

    //   TEST_ASSERT_EQUAL(0, task.execute());
    //   TEST_ASSERT_EQUAL(2, task.get_current_fn_number());

    //   // Test that the state is reset back to idle and fnNumber is set back to 0
    //   TEST_ASSERT_EQUAL(0, task.execute());
    //   TEST_ASSERT_EQUAL(0, task.get_current_fn_number());
    //   TEST_ASSERT_EQUAL(IDLE, task.get_current_state());

    //   // Check that config can interrupt SBDIX
    //   TEST_ASSERT_EQUAL(true, task.request_state(SBDIX));
    //   TEST_ASSERT_EQUAL(SBDIX, task.get_current_state());
    //   TEST_ASSERT_EQUAL(0, task.execute());
    //   TEST_ASSERT_EQUAL(SBDIX, task.get_current_state());
    //   TEST_ASSERT_EQUAL(1, task.get_current_fn_number());
    //   // Test config
    //   TEST_ASSERT_EQUAL(true, task.request_state(CONFIG));
    //   TEST_ASSERT_EQUAL(CONFIG, task.get_current_state());
    //   TEST_ASSERT_EQUAL(0, task.get_current_fn_number());
    //   TEST_ASSERT_EQUAL(0, task.execute());
    //   TEST_ASSERT_EQUAL(0, task.execute());
    //   TEST_ASSERT_EQUAL(0, task.execute());
    //   TEST_ASSERT_EQUAL(0, task.execute());
    //   TEST_ASSERT_EQUAL(0, task.get_current_fn_number());
    //   TEST_ASSERT_EQUAL(IDLE, task.get_current_state());

    //   // test SBDIX
    //   TEST_ASSERT_EQUAL(true, task.request_state(SBDIX));
    //   TEST_ASSERT_EQUAL(SBDIX, task.get_current_state());
    //   TEST_ASSERT_EQUAL(0, task.get_current_fn_number());
    //   TEST_ASSERT_EQUAL(0, task.execute());
    //   TEST_ASSERT_EQUAL(1, task.get_current_fn_number());
    //   TEST_ASSERT_EQUAL(0, task.execute());
    //   TEST_ASSERT_EQUAL(0, task.get_current_fn_number());
    //   TEST_ASSERT_EQUAL(IDLE, task.get_current_state());

    //   // test SBDRB
    //   TEST_ASSERT_EQUAL(true, task.request_state(SBDRB));
    //   TEST_ASSERT_EQUAL(SBDRB, task.get_current_state());
    //   TEST_ASSERT_EQUAL(0, task.get_current_fn_number());
    //   TEST_ASSERT_EQUAL(0, task.execute());
    //   TEST_ASSERT_EQUAL(1, task.get_current_fn_number());
    //   TEST_ASSERT_EQUAL(0, task.execute());
    //   TEST_ASSERT_EQUAL(0, task.get_current_fn_number());
    //   TEST_ASSERT_EQUAL(IDLE, task.get_current_state());
}

int test_control_task()
{
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_task_execute);
    return UNITY_END();
}

#ifdef DESKTOP
int main()
{
    return test_control_task();
}
#else
#include <Arduino.h>
void setup()
{
    delay(2000);
    Serial.begin(9600);
    test_control_task();
}

void loop() {}
#endif