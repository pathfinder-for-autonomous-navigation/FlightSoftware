#include <unity.h>
#include <fsw/FCCode/PropController.hpp>
#include "../StateFieldRegistryMock.hpp"
class PropTestFixture 
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

    PropTestFixture(prop_state_t initial_state = prop_state_t::disabled)
    {
       prop_controller = std::make_unique<PropController>(registry, 0);
       prop_state_fp = registry.find_writable_field_t<unsigned int>("prop.state");
       fire_cycle_fp = registry.find_writable_field_t<unsigned int>("prop.fire_cycle");
       sched_valve1_fp = registry.find_writable_field_t<unsigned int>("prop.sched_valve1");
       sched_valve2_fp = registry.find_writable_field_t<unsigned int>("prop.sched_valve2");
       sched_valve3_fp = registry.find_writable_field_t<unsigned int>("prop.sched_valve3");
       sched_valve4_fp = registry.find_writable_field_t<unsigned int>("prop.sched_valve4");
    }

    inline void fixture_set_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4, unsigned int ctrl_cycles_from_now)
    {
      sched_valve1_fp->set(v1);
      sched_valve2_fp->set(v2);
      sched_valve3_fp->set(v3);
      sched_valve4_fp->set(v4);
      fire_cycle_fp->set(ctrl_cycles_from_now);
    }

    // Step forward the state machine by num control cycle.
    inline void step(size_t num=1)
    {
      for (size_t i = 0; i < num; ++i)
        prop_controller->execute();

      if (PropulsionSystem.is_firing())
      {
        for (size_t i = 0; i < 4; ++i)
        {
          // decrement each schedule as if we were thrust valve loop
          if (Tank2.schedule[i] > PAN::control_cycle_time_ms)
            Tank2.schedule[i] -= PAN::control_cycle_time_ms;
          else
            Tank2.schedule[i] = 0;  
        }
      }
    }

    // Step forward a number of control cycles equivalent to ms
    inline void step_time(size_t num_ms)
    {
      size_t num_cc = num_ms/PAN::control_cycle_time_ms;
      step(num_cc);
    }

    inline void set_state(prop_state_t state)
    {
      prop_state_fp->set(static_cast<unsigned int>(state));
    }

    inline void check_state(prop_state_t expected)
    {
      TEST_ASSERT_EQUAL(expected, prop_state_fp->get());
    }

    inline void check_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4, unsigned int ctrl_cycles_from_now)
    {
      TEST_ASSERT_EQUAL(v1, sched_valve1_fp->get());
      TEST_ASSERT_EQUAL(v2, sched_valve2_fp->get());
      TEST_ASSERT_EQUAL(v3, sched_valve3_fp->get());
      TEST_ASSERT_EQUAL(v4, sched_valve4_fp->get());
      TEST_ASSERT_EQUAL(fire_cycle_fp->get(), ctrl_cycles_from_now);
    }

};

void test_initialization()
{
  PropTestFixture tf;
  tf.check_state(prop_state_t::disabled);
}

void test_disable()
{
  PropTestFixture tf;
  // Prop should remain in disabled state until manually set to some other state
  tf.fixture_set_schedule(200, 300, 400,500, 21);
  // Firing time is set 5 control cycles from now but we should not fire since
  // state is still disabled
  tf.check_state(prop_state_t::disabled);
  for (size_t i = 0; i < 22; ++i)
  {
    tf.step();
    tf.check_state(prop_state_t::disabled);
  }
}

void test_illegal_schedule()
{
  PropTestFixture tf;
  tf.set_state(prop_state_t::idle);
  // Prop should ignore if requested firing time is less than 21 cycles into the future
  // since we will not have time to pressurize Tank1
  unsigned int cycles_until_fire = PropState_Pressurizing::num_cycles_needed() - 1;
  tf.fixture_set_schedule(200, 400, 800, 100, cycles_until_fire);
  tf.step();
  // State should remain in idle
  tf.check_state(prop_state_t::idle);
  tf.step(cycles_until_fire - PropState_Idle::num_cycles_within_firing_to_pressurize);
  tf.check_state(prop_state_t::idle);
}

void test_idle_to_pressurize()
{
  PropTestFixture tf;
  // Prop may only switch to pressurizing state when in IDLE state
  tf.set_state(prop_state_t::idle);
  for (size_t i = 0; i < 5; ++i)
  {
    tf.step();
    // Stay in IDLE until received schedule
    tf.check_state(prop_state_t::idle);
  }
  unsigned int cycles_until_fire = PropState_Pressurizing::num_cycles_needed();
  tf.fixture_set_schedule(200, 400, 12, 800, cycles_until_fire);
  tf.step();
  // State should be pressurizing
  tf.check_state(prop_state_t::pressurizing);
  // Schedule should be set and cycle_to_fire should be decremented
  tf.check_schedule(200, 400, 12, 800, PropState_Pressurizing::num_cycles_needed() - 1);
}

void test_presurize_to_await_firing()
{
  PropTestFixture tf;
  tf.set_state(prop_state_t::idle);
  unsigned int cycles_until_fire = PropState_Pressurizing::num_cycles_needed();
  tf.fixture_set_schedule(200, 200, 200, 200, cycles_until_fire);
  // we may finish pressurizing earlier than 20 cycles, in this case, the 
  // state should be in await_firing until the actual firing cycle (21)
  tf.step();
  tf.check_state(prop_state_t::pressurizing);
  tf.step(PropState_Pressurizing::num_cycles_needed() - 1);
  tf.check_state(prop_state_t::await_firing);
}

void test_firing_to_idle()
{
  PropTestFixture tf;
  tf.set_state(prop_state_t::idle);
  unsigned int cycles_until_fire = PropState_Pressurizing::num_cycles_needed();
  tf.fixture_set_schedule(700, 200, 200, 800, cycles_until_fire);
  tf.step(cycles_until_fire);
  tf.check_state(prop_state_t::await_firing);
  // For 800/140 control cycles, we should be firing
  unsigned int cycles_firing = (800/PAN::control_cycle_time_ms);
  for(size_t i = 0; i < cycles_firing + 1; ++i)
  {
    tf.step();
    // Check the schedule to see if we are done
    tf.check_state(prop_state_t::firing);
  }
  tf.step();
  // On the next control cycle, we should be back to idle
  tf.check_state(prop_state_t::idle);
}

void test_pressurizing()
{
  // Test that the prop system is in the state associated with pressurizing
  PropTestFixture tf;
  tf.set_state(prop_state_t::idle);
  unsigned int cycles_until_fire = PropState_Pressurizing::num_cycles_needed();
  tf.fixture_set_schedule(700, 200, 200, 800, cycles_until_fire);
  tf.step();
  tf.check_state(prop_state_t::pressurizing);
  //TODO: check the prop system states associated with pressurizing
}

void test_pressurize_late()
{
  PropTestFixture tf;
  tf.set_state(prop_state_t::idle);
  unsigned int cycles_until_fire = PropState_Pressurizing::num_cycles_needed() + 2*PropState_Idle::num_cycles_within_firing_to_pressurize;
  tf.fixture_set_schedule(430, 23, 122, 33, cycles_until_fire);
  // Don't start pressurizing until we are within like 50 cycles of firing time
  tf.step();
  tf.check_state(prop_state_t::idle);
  tf.step(PropState_Idle::num_cycles_within_firing_to_pressurize);
  tf.check_state(prop_state_t::idle);
  tf.step();
  tf.check_state(prop_state_t::pressurizing);
}

void test_pressurize_fail()
{
  PropTestFixture tf;
  tf.set_state(prop_state_t::idle);
  tf.fixture_set_schedule(700, 200, 200, 800, 25);
  // TODO: test the condition where we pressurized more than 20 times
}

void test_await_firing()
{
  PropTestFixture tf;
  tf.set_state(prop_state_t::idle);
  unsigned int cycles_until_fire = PropState_Pressurizing::num_cycles_needed() + 5;
  tf.fixture_set_schedule(700, 200, 200, 800, cycles_until_fire);
  tf.step(cycles_until_fire - 5);
  tf.check_state(prop_state_t::await_firing);
  tf.step(5);
  tf.check_state(prop_state_t::await_firing);

  // TODO: check state associated with await_firing
  tf.step();
  tf.check_state(prop_state_t::firing);
}

void test_firing()
{
  PropTestFixture tf;
  tf.set_state(prop_state_t::idle);
  unsigned int cycles_until_fire = PropState_Pressurizing::num_cycles_needed() + 5;
  tf.fixture_set_schedule(700, 200, 200, 800, cycles_until_fire);
  tf.step(cycles_until_fire + 1);
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
    RUN_TEST(test_pressurize_late);
    RUN_TEST(test_pressurize_fail);
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
