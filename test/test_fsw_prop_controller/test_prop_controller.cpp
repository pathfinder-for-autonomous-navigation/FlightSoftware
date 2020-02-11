#include <unity.h>
#include <fsw/FCCode/PropController.hpp>
#include "../StateFieldRegistryMock.hpp"

class TestFixture 
{
  public:
    StateFieldRegistryMock registry;
    std::unique_ptr<PropController> prop_controller;
    // Output state field from prop controller
    WritableStateField<unsigned int>* prop_state_fp;
    WritableStateField<unsigned int>* fire_cycle_fp;
    WritableStateField<unsigned int>* sched_valve1_fp;
    WritableStateField<unsigned int>* sched_valve2_fp;
    WritableStateField<unsigned int>* sched_valve3_fp;
    WritableStateField<unsigned int>* sched_valve4_fp;

    TestFixture(prop_state_t initial_state = prop_state_t::disabled)
    {
       prop_controller = std::make_unique<PropController>(registry, 0);
       prop_state_fp = registry.find_writable_field_t<unsigned int>("prop.state");
       fire_cycle_fp = registry.find_writable_field_t<unsigned int>("prop.fire_cycle");
       sched_valve1_fp = registry.find_writable_field_t<unsigned int>("prop.sched_valve1");
       sched_valve2_fp = registry.find_writable_field_t<unsigned int>("prop.sched_valve2");
       sched_valve3_fp = registry.find_writable_field_t<unsigned int>("prop.sched_valve3");
       sched_valve4_fp = registry.find_writable_field_t<unsigned int>("prop.sched_valve4");
    }

    // Step forward the state machine by num control cycle.
    inline void step(size_t num=1)
    {
      for (size_t i = 0; i < num; ++i)
        prop_controller->execute();
    }

    inline void set_state(prop_state_t state)
    {
      prop_state_fp->set(static_cast<unsigned int>(state));
    }

    inline void check_state(prop_state_t expected)
    {
      TEST_ASSERT_EQUAL(prop_state_fp->get(), expected);
    }

    inline void check_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4, unsigned int ctrl_cycles_from_now)
    {
      TEST_ASSERT_EQUAL(sched_valve1_fp->get(), v1);
      TEST_ASSERT_EQUAL(sched_valve2_fp->get(), v2);
      TEST_ASSERT_EQUAL(sched_valve3_fp->get(), v3);
      TEST_ASSERT_EQUAL(sched_valve4_fp->get(), v4);
      TEST_ASSERT_EQUAL(fire_cycle_fp->get(), ctrl_cycles_from_now);
    }

};

void test_initialization()
{
  TestFixture tf;
  tf.check_state(prop_state_t::disabled);
}

void test_disable()
{
  TestFixture tf;
  // Prop should remain in disabled state until manually set to some other state
  tf.prop_controller->set_schedule(200, 300, 400,500, 21);
  // Firing time is set 5 control cycles from now but we should not fire since
  // state is still disabled
  for (size_t i = 0; i < 22; ++i)
  {
    tf.step();
    tf.check_state(prop_state_t::disabled);
  }
}

void test_illegal_schedule()
{
  TestFixture tf;
  tf.set_state(prop_state_t::idle);
  // Prop should ignore if requested firing time is less than 21 cycles into the future
  // since we will not have time to pressurize Tank1
  tf.prop_controller->set_schedule(200, 400, 800, 100, 20);
  tf.step();
  // Schedule should not have been set
  tf.check_schedule(0, 0, 0, 0, 0);
  // State should remain in idle
  tf.check_state(prop_state_t::idle);
}

void test_idle_to_pressurize()
{
  TestFixture tf;
  // Prop may only switch to pressurizing state when in IDLE state
  tf.set_state(prop_state_t::idle);
  for (size_t i = 0; i < 5; ++i)
  {
    tf.step();
    // Stay in IDLE until received schedule
    tf.check_state(prop_state_t::idle);
  }
  tf.prop_controller->set_schedule(200, 400, 12, 800, 21);
  tf.step();
  // Schedule should be set and cycle_to_fire should be decremented
  tf.check_schedule(200, 400, 12, 800, 20);
  // State should be pressurizing
  tf.check_state(prop_state_t::pressurizing);
}

void test_presurize_to_await_firing()
{
  TestFixture tf;
  tf.set_state(prop_state_t::idle);
  tf.prop_controller->set_schedule(200, 200, 200, 200, 21);
  // we may finish pressurizing earlier than 20 cycles, in this case, the 
  // state should be in await_firing until the actual firing cycle (21)
  tf.step(20);
  tf.check_state(prop_state_t::await_firing);
  tf.step();
  tf.check_state(prop_state_t::firing);
}

void test_firing_to_idle()
{
  TestFixture tf;
  tf.set_state(prop_state_t::idle);
  tf.prop_controller->set_schedule(700, 200, 200, 800, 22);
  tf.step(21);
  tf.check_state(prop_state_t::await_firing);
  // For 800/140 control cycles, we should be firing
  for(size_t i = 0; i < 800/140; ++i)
  {
    tf.step();
    tf.check_state(prop_state_t::firing);
  }
  // On the next control cycle, we should be back to idle
  tf.check_state(prop_state_t::idle);
}

void test_pressurizing()
{
  // Test that the prop system is in the state associated with pressurizing
  TestFixture tf;
  tf.set_state(prop_state_t::idle);
  tf.prop_controller->set_schedule(700, 200, 200, 800, 22);
  tf.step();
  tf.check_state(prop_state_t::pressurizing);
  //TODO: check the prop system states associated with pressurizing
}

void test_await_firing()
{
  TestFixture tf;
  tf.set_state(prop_state_t::idle);
  tf.prop_controller->set_schedule(700, 200, 200, 800, 25);
  tf.step(20);
  // we should be in await_firing for at least 25 - 20 cycles = 5
  tf.check_state(prop_state_t::await_firing);
  tf.step(4);
  tf.check_state(prop_state_t::await_firing);

  // TODO: check state associated with await_firing
  tf.step();
  tf.check_state(prop_state_t::firing);
}

void test_firing()
{
  TestFixture tf;
  tf.set_state(prop_state_t::idle);
  tf.prop_controller->set_schedule(700, 200, 200, 800, 25);
  tf.step(25);
  tf.check_state(prop_state_t::firing);
  // TODO: check state associated with firing
}

int test_prop_controller() {
    UNITY_BEGIN();
    RUN_TEST(test_initialization);
    RUN_TEST(test_disable);
    RUN_TEST(test_illegal_schedule);
    RUN_TEST(test_idle_to_pressurize);
    RUN_TEST(test_presurize_to_await_firing);
    RUN_TEST(test_firing_to_idle);
    RUN_TEST(test_pressurizing);
    RUN_TEST(test_await_firing);
    RUN_TEST(test_firing);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_prop_controller();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_prop_controller();
}

void loop() {}
#endif
