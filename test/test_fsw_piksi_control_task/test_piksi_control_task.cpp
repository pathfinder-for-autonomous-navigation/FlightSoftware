#include <StateFieldRegistry.hpp>
#include "../../src/FCCode/PiksiControlTask.hpp"
#include <unity.h>

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

    //expect 1 cuz no bytes can be read lmao
    TEST_ASSERT_EQUAL(1,task.execute());
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