#include <fsw/FCCode/QuakeControlTask.h>

#include <common/StateFieldRegistry.hpp>
#include <unity.h>

using namespace Devices;

/**
 * Helper function, executes task and checks that the state after execution
 * is [expectedState] at fnNum [expectedFnNum]
 */
void exec_check(QuakeControlTask& task, int expectedState, int expectedFnNum)
{
  TEST_ASSERT_EQUAL(Devices::OK, task.execute());
  TEST_ASSERT_EQUAL(expectedState, task.get_current_state());
  TEST_ASSERT_EQUAL(expectedFnNum, task.get_current_fn_number());
}


void test_task_initialization()
{
  StateFieldRegistry registry;
  QuakeControlTask task(registry);
  // Check that things are initialized correctly
  exec_check(task, IDLE, 0);
}

// Check that we can request_states when IDLE and fnNum == 0
void test_request_state()
{
  StateFieldRegistry registry;
  QuakeControlTask task(registry);
  TEST_ASSERT_EQUAL(true, task.request_state(SBDIX));
  exec_check(task, SBDIX, 1);
}

  // Check that config can interrupt any state
void test_config_can_interrupt()
{
  StateFieldRegistry registry;
  QuakeControlTask task(registry);
  task.set_downlink_msg("Blah", 4);
  TEST_ASSERT_EQUAL(true, task.request_state(SBDWB));
  exec_check(task, SBDWB, 1);

  TEST_ASSERT_EQUAL(true, task.request_state(CONFIG));
  TEST_ASSERT_EQUAL(CONFIG, task.get_current_state());
  exec_check(task, CONFIG, 1);
}

// Check that idle can interrupt any state
void test_idle_can_interrupt()
{
  StateFieldRegistry registry;
  QuakeControlTask task(registry);
  TEST_ASSERT_EQUAL(true, task.request_state(SBDIX));
  exec_check(task, SBDIX, 1);

  TEST_ASSERT_EQUAL(true, task.request_state(IDLE));
  exec_check(task, IDLE, 0);
}

// Check that calling execute() when IDLE does not change anything
void test_call_exec_when_idle()
{
  StateFieldRegistry registry;
  QuakeControlTask task(registry);
  exec_check(task, IDLE, 0);
  exec_check(task, IDLE, 0);
  exec_check(task, IDLE, 0);
}

// Check that sbdwb with a zero length message should fail and not change fnNumber
void test_sbdwb_fail()
{
  StateFieldRegistry registry;
  QuakeControlTask task(registry);
  TEST_ASSERT_EQUAL(true, task.request_state(SBDWB));
  TEST_ASSERT_EQUAL(Devices::WRONG_LENGTH, task.execute());
  TEST_ASSERT_EQUAL(IDLE, task.get_current_state());
  TEST_ASSERT_EQUAL(0, task.get_current_fn_number());
}

// Test that SBDWB completes when a message is loaded
void test_sbdwb()
{
  StateFieldRegistry registry;
  QuakeControlTask task(registry);
  task.set_downlink_msg("hello", 5);
  exec_check(task, IDLE, 0);

  TEST_ASSERT_EQUAL(true, task.request_state(SBDWB));
  exec_check(task, SBDWB, 1);
  exec_check(task, SBDWB, 2);
  exec_check(task, IDLE, 0);
}

// Test that SBDRB completes in two cycles
void test_sbdrb()
{
  StateFieldRegistry registry;
  QuakeControlTask task(registry);
  TEST_ASSERT_EQUAL(true, task.request_state(SBDRB));
  exec_check(task, SBDRB, 1); 
  exec_check(task, IDLE, 0);
}

// Test that SBDIX completes in two cycles
void test_sbdix()
{
  StateFieldRegistry registry;
  QuakeControlTask task(registry);
  TEST_ASSERT_EQUAL(true, task.request_state(SBDIX));
  exec_check(task, SBDIX, 1); 
  exec_check(task, IDLE, 0);
}

// Test that CONFIG completes in two cycles
void test_config()
{
  StateFieldRegistry registry;
  QuakeControlTask task(registry);
  TEST_ASSERT_EQUAL(true, task.request_state(CONFIG));
  exec_check(task, CONFIG, 1); 
  exec_check(task, CONFIG, 2); 
  exec_check(task, CONFIG, 3); 
  exec_check(task, IDLE, 0);
}

// Check that changing state is not ok and does not change the fn number or state
void test_sbdwb_noint()
{
  StateFieldRegistry registry;
  QuakeControlTask task(registry);
  TEST_ASSERT_EQUAL(true, task.request_state(CONFIG));
  exec_check(task, CONFIG, 1);
  TEST_ASSERT_EQUAL(CONFIG, task.get_current_state());
}

void test_task_execute()
{
  RUN_TEST(test_sbdwb_fail);
  RUN_TEST(test_sbdwb);
  RUN_TEST(test_sbdrb);
  RUN_TEST(test_sbdix);
  RUN_TEST(test_config);
}

int test_control_task()
{
  UNITY_BEGIN();
  RUN_TEST(test_task_initialization);
  RUN_TEST(test_request_state);
  RUN_TEST(test_config_can_interrupt);
  RUN_TEST(test_idle_can_interrupt);
  RUN_TEST(test_sbdwb_noint);
  RUN_TEST(test_call_exec_when_idle);
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