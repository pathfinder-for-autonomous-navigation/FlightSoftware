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
    std::array<double, 3> pos = {1000.0, 2000.0, 3000.0};
    std::array<double, 3> vel = {4000.0, 5000.0, 6000.0};
    std::array<double, 3> baseline = {7000.0, 8000.0, 9000.0};


    task.piksi.set_read_return(2);
    task.execute();
    TEST_ASSERT_EQUAL(DATA_ERROR, task.get_current_state());

    task.piksi.set_read_return(3);
    task.execute();
    TEST_ASSERT_EQUAL(NO_DATA, task.get_current_state());

    unsigned int tow = 100;
    unsigned int bad_tow = 200;
    task.piksi.set_read_return(1);
    task.piksi.set_gps_time(bad_tow);
    task.piksi.set_pos_ecef(tow, pos, 4);
    task.piksi.set_vel_ecef(tow, vel);
    task.piksi.set_baseline_ecef(tow, baseline);
    task.piksi.set_baseline_flag(1);
    task.execute();
    //should error out because of time inconsistency
    TEST_ASSERT_EQUAL(SYNC_ERROR, task.get_current_state());

    tow = 200;
    task.piksi.set_read_return(1);
    task.piksi.set_gps_time(tow);
    task.piksi.set_pos_ecef(tow, pos, 3);
    task.piksi.set_vel_ecef(tow, vel);
    task.piksi.set_baseline_ecef(tow, baseline);
    task.piksi.set_baseline_flag(1);
    task.execute();
    //times agree, but insufficient nsat
    TEST_ASSERT_EQUAL(NSAT_ERROR, task.get_current_state());
    
    tow = 200;
    task.piksi.set_read_return(1);
    task.piksi.set_gps_time(tow);
    task.piksi.set_pos_ecef(tow, pos, 4);
    task.piksi.set_vel_ecef(tow, vel);
    task.piksi.set_baseline_ecef(tow, baseline);
    task.piksi.set_baseline_flag(1);
    task.execute();
    //times should now agree, and be in baseline
    TEST_ASSERT_EQUAL(FIXED_RTK, task.get_current_state());

    tow = 200;
    task.piksi.set_read_return(1);
    task.piksi.set_gps_time(tow);
    task.piksi.set_pos_ecef(tow, pos, 4);
    task.piksi.set_vel_ecef(tow, vel);
    task.piksi.set_baseline_ecef(tow, baseline);
    task.piksi.set_baseline_flag(0);
    task.execute();
    //float rtk test
    TEST_ASSERT_EQUAL(FLOAT_RTK, task.get_current_state());

    
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