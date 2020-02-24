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
       PropulsionSystem.reset();
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
    inline void step_time(size_t num_ms) {
        size_t num_cc = num_ms / PAN::control_cycle_time_ms;
        step(num_cc);
    }

    inline void set_state(prop_state_t state) {
        prop_state_fp->set(static_cast<unsigned int>(state));
    }

    inline void check_state(prop_state_t expected) {
        TEST_ASSERT_EQUAL(expected, prop_state_fp->get());
    }

    inline void check_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4,
                               unsigned int ctrl_cycles_from_now) {
        TEST_ASSERT_EQUAL(v1, sched_valve1_fp->get());
        TEST_ASSERT_EQUAL(v2, sched_valve2_fp->get());
        TEST_ASSERT_EQUAL(v3, sched_valve3_fp->get());
        TEST_ASSERT_EQUAL(v4, sched_valve4_fp->get());
        TEST_ASSERT_EQUAL(fire_cycle_fp->get(), ctrl_cycles_from_now);
    }

    // keep stepping until the state changes and return the number of steps it took
    // return 0 if we've executed for max_cycles
    inline size_t execute_until_state_change(size_t max_cycles=2*PropState_Pressurizing::num_cycles_needed()) {
        size_t num_steps = 0;
        unsigned int current_state = prop_state_fp->get();
        while (current_state == prop_state_fp->get() && num_steps < max_cycles) {
            step();
            ++num_steps;
        }
        return (num_steps < max_cycles) ? num_steps : 6969696969;
    }

};

extern size_t g_fake_pressure_cycle_count;

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
  // Prop should ignore if requested firing time is less than 20 pressurizing cycles + 1 control cycle into the future
  tf.fixture_set_schedule(200, 400, 800, 100, PropState_Pressurizing::num_cycles_needed());
  tf.step();
  // State should remain in idle
  tf.check_state(prop_state_t::idle);
}

void test_idle_to_pressurizing()
{
    PropTestFixture tf;
    tf.set_state(prop_state_t::idle);
    tf.fixture_set_schedule(200, 400, 800, 100, PropState_Pressurizing::num_cycles_needed() + 1);
    tf.step();
    // Go immediately into pressurizing
    tf.check_state(prop_state_t::pressurizing);
}

void test_idle_to_await_pressurize()
{
    PropTestFixture tf;
    tf.set_state(prop_state_t::idle);
    tf.fixture_set_schedule(200, 400, 800, 100, PropState_Pressurizing::num_cycles_needed() + 2);
    tf.step();
    // Go into await_pressurizing because we have more than enough time
    tf.check_state(prop_state_t::await_pressurizing);
}

void test_await_pressurize_to_pressurize()
{
    PropTestFixture tf;
    tf.set_state(prop_state_t::idle);
    tf.fixture_set_schedule(200, 400, 12, 800, PropState_Pressurizing::num_cycles_needed() + 10);
    tf.step();
    tf.check_state(prop_state_t::await_pressurizing);
    TEST_ASSERT_EQUAL(9, tf.execute_until_state_change()); // goes into pressurizing after 9 cycles in await_pressurizing
    tf.check_state(prop_state_t::pressurizing);
}

void test_pressurize_to_firing()
{
    // We can go directly into firing if it happens that it took us exactly 20 pressurizing cycles to pressurize
    g_fake_pressure_cycle_count = 20;
    PropTestFixture tf;
    tf.set_state(prop_state_t::idle);
    tf.fixture_set_schedule(200, 400, 12, 800, PropState_Pressurizing::num_cycles_needed() + 1);
    tf.step();
    tf.check_state(prop_state_t::pressurizing);
    // consumes the max possible number of cycles
    TEST_ASSERT_EQUAL(PropState_Pressurizing::num_cycles_needed(), tf.execute_until_state_change());
    tf.check_state(prop_state_t::firing);
}

void test_pressurize_to_await_firing() {
    g_fake_pressure_cycle_count = 19; // finish 1 pressurizing cycle early
    PropTestFixture tf;
    tf.set_state(prop_state_t::idle);
    tf.fixture_set_schedule(200, 200, 200, 200, PropState_Pressurizing::num_cycles_needed() + 1);
    tf.step();
    tf.check_state(prop_state_t::pressurizing);
    size_t fake_cycles = PropState_Pressurizing::num_cycles_needed() - PropState_Pressurizing::ctrl_cycles_per_pressurizing_cycle;
    TEST_ASSERT_EQUAL(fake_cycles, tf.execute_until_state_change());
    tf.check_state(prop_state_t::await_firing);
}

void test_pressurize_to_await_firing_mid_cycle()
{
    g_fake_pressure_cycle_count = 19; // finish 1 pressurizing cycle early
    PropTestFixture tf;
    tf.set_state(prop_state_t::idle);
    tf.fixture_set_schedule(200, 200, 200, 200, PropState_Pressurizing::num_cycles_needed() + 1);
    tf.step();
    tf.check_state(prop_state_t::pressurizing);
    size_t fake_cycles = PropState_Pressurizing::num_cycles_needed() - PropState_Pressurizing::ctrl_cycles_per_pressurizing_cycle - 1;
    TEST_ASSERT_EQUAL(fake_cycles, tf.execute_until_state_change());
    tf.check_state(prop_state_t::await_firing);
}

void test_pressurize_firing_boundary()
{
    g_fake_pressure_cycle_count = 19; // finish 1 pressurizing cycle early
    PropTestFixture tf;
    tf.set_state(prop_state_t::idle);
    tf.fixture_set_schedule(200, 200, 200, 200, PropState_Pressurizing::num_cycles_needed() + 1);
    tf.step();
    tf.check_state(prop_state_t::pressurizing);
    size_t fake_cycles = PropState_Pressurizing::num_cycles_needed() - PropState_Pressurizing::ctrl_cycles_per_pressurizing_cycle + 1;
    TEST_ASSERT_EQUAL(fake_cycles, tf.execute_until_state_change());
    tf.check_state(prop_state_t::firing);
}

void test_pressurizing()
{
    // Test that the prop system is in the state associated with pressurizing
    PropTestFixture tf;
    tf.set_state(prop_state_t::idle);
    unsigned int cycles_until_fire = PropState_Pressurizing::num_cycles_needed() + 1;
    tf.fixture_set_schedule(700, 200, 200, 800, cycles_until_fire);
    tf.step();
    tf.check_state(prop_state_t::pressurizing);
    TEST_ASSERT_TRUE(Tank1.is_valve_open(0));
    tf.execute_until_state_change();
    TEST_ASSERT_FALSE(Tank1.is_valve_open(0));
    TEST_ASSERT_FALSE(Tank1.is_valve_open(1));
    TEST_ASSERT_FALSE(Tank1.is_valve_open(2));
    TEST_ASSERT_FALSE(Tank1.is_valve_open(3));
    tf.check_state(prop_state_t::await_firing);
}

void test_pressurize_fail(){
    g_fake_pressure_cycle_count = 21;
    PropTestFixture tf;
    tf.set_state(prop_state_t::idle);
    tf.fixture_set_schedule(700, 200, 200, 800, PropState_Pressurizing::num_cycles_needed()+1);
    tf.step();
    tf.check_state(prop_state_t::pressurizing);
    TEST_ASSERT_EQUAL(PropState_Pressurizing::num_cycles_needed(), tf.execute_until_state_change());
    // TODO: sure we want to disable?
    tf.check_state(prop_state_t::disabled);
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
  TEST_ASSERT_TRUE(PropulsionSystem.is_firing())
  TEST_ASSERT_TRUE(Tank2.is_valve_open(0));
  TEST_ASSERT_TRUE(Tank2.is_valve_open(1));
  TEST_ASSERT_TRUE(Tank2.is_valve_open(2));
  TEST_ASSERT_TRUE(Tank2.is_valve_open(3));
  tf.step();
  // all valves should be opened
  
  // TODO: check state associated with firing
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
    TEST_ASSERT_TRUE(PropulsionSystem.is_firing())
  }
  tf.step();
  // On the next control cycle, we should be back to idle
  TEST_ASSERT_FALSE(PropulsionSystem.is_firing())
  tf.check_state(prop_state_t::idle);
}

int test_prop_controller() {
    UNITY_BEGIN();
    RUN_TEST(test_initialization);
    RUN_TEST(test_disable);
    RUN_TEST(test_illegal_schedule);
    RUN_TEST(test_idle_to_pressurizing);
    RUN_TEST(test_idle_to_await_pressurize);
    RUN_TEST(test_await_pressurize_to_pressurize);
    RUN_TEST(test_pressurize_to_firing);
    RUN_TEST(test_pressurize_to_await_firing);
    RUN_TEST(test_pressurize_to_await_firing_mid_cycle);
    RUN_TEST(test_pressurize_firing_boundary);
    RUN_TEST(test_firing_to_idle);
    RUN_TEST(test_pressurizing);
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
