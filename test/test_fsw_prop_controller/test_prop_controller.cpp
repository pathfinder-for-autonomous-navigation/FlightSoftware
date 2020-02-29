#include <unity.h>

#include <fsw/FCCode/PropController.hpp>
#include "../StateFieldRegistryMock.hpp"
extern size_t g_fake_pressure_cycle_count;

class PropTestFixture {
public:
    StateFieldRegistryMock registry;
    std::unique_ptr <PropController> prop_controller;
    // Output state field from prop controller
    WritableStateField<unsigned int> *prop_state_fp;
    WritableStateField<unsigned int> *fire_cycle_fp;
    WritableStateField<unsigned int> *sched_valve1_fp;
    WritableStateField<unsigned int> *sched_valve2_fp;
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
        g_fake_pressure_cycle_count = 16;
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

    // Keep stepping until the state changes (or until we have step max_cycles steps)
    // Return the number of steps taken or return 6969696969 if there has been no state change after max_cycles steps
    inline size_t execute_until_state_change(size_t max_cycles=2*PropState_Pressurizing::num_cycles_needed()) {
        size_t num_steps = 0;
        unsigned int current_state = prop_state_fp->get();
        while (current_state == prop_state_fp->get() && num_steps < max_cycles) {
            ++num_steps;
            step();
        }
        return (num_steps < max_cycles) ? num_steps : 6969696969;
    }

};

void test_initialization()
{
  PropTestFixture tf;
  tf.check_state(prop_state_t::disabled);
}

// Test that PropController remains in the disabled state despite the existence of a valid schedule
void test_disable()
{
  PropTestFixture tf;
  // Prop should remain in disabled state until manually set to some other state
  tf.fixture_set_schedule(200, 300, 400,500, PropState_Pressurizing::num_cycles_needed() + 1);
  // Firing time is set 5 control cycles from now but we should not fire since
  // state is still disabled
  tf.check_state(prop_state_t::disabled);
  for (size_t i = 0; i < PropState_Pressurizing::num_cycles_needed() ; ++i)
  {
    tf.step();
    tf.check_state(prop_state_t::disabled);
  }
}

// Test that Prop ignores a schedule with fewer than PropState_Pressurizing::num_cycles_needed() + 1 cycles
// Test that Prop ignores a schedule if a value exceeds 999
void test_illegal_schedule()
{
  PropTestFixture tf;
  tf.set_state(prop_state_t::idle);
  // Prop should ignore if requested firing time is less than 20 pressurizing cycles + 1 control cycle into the future
  tf.fixture_set_schedule(200, 400, 800, 100, PropState_Pressurizing::num_cycles_needed());
  tf.step();
  // State should remain in idle because fire cycle too soon
  tf.check_state(prop_state_t::idle);
  tf.fixture_set_schedule(200, 400, 800, 1000, PropState_Pressurizing::num_cycles_needed() + 1);
  tf.step();
  // State should remain in idle because valve 4 is scheduled for 1000
  tf.check_state(prop_state_t::idle);
}

// Test that a schedule with fire_cycle set to PropState_Pressurizing::num_cycles_needed() + 1 and all valve
// schedules less than 1000 should be considered valid
// Test that we should go directly into pressurizing (as oppose to await_pressurizing)
void test_idle_to_pressurizing()
{
    PropTestFixture tf;
    tf.set_state(prop_state_t::idle);
    tf.fixture_set_schedule(200, 400, 800, 999, PropState_Pressurizing::num_cycles_needed() + 1);
    tf.step();
    // Go immediately into pressurizing
    tf.check_state(prop_state_t::pressurizing);
}

// Test that a schedule with fire_cycle set to PropState_Pressurizing::num_cycles_needed() + 2 will cause
// Prop to enter await_pressurizing for 1 cycle before entering pressurizing
void test_idle_to_await_pressurize()
{
    PropTestFixture tf;
    tf.set_state(prop_state_t::idle);
    tf.fixture_set_schedule(200, 400, 800, 100, PropState_Pressurizing::num_cycles_needed() + 2);
    tf.step();
    // Go into await_pressurizing because we have more than enough time
    tf.check_state(prop_state_t::await_pressurizing);
    tf.step();
    tf.check_state(prop_state_t::pressurizing);
}

// Test that Prop waits in await_pressurizing if a schedule is set with fire_cycle exceeding
// PropState_Pressurizing::num_cycles_needed()
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

// Test that we pressurize for 19 cycles before transitioning to await_firing
void test_pressurize_to_await_firing()
{
    PropTestFixture tf;
    g_fake_pressure_cycle_count = 19; // finish 1 pressurizing cycle early
    tf.set_state(prop_state_t::idle);
    tf.fixture_set_schedule(200, 200, 200, 200, PropState_Pressurizing::num_cycles_needed() + 1);
    tf.step();
    tf.check_state(prop_state_t::pressurizing);
    // Expect to execute 18 full pressurizing cycles before transitioning on the 19th
    unsigned int expected_cycles =
            (PropController::max_pressurizing_cycles - 2) * PropState_Pressurizing::ctrl_cycles_per_pressurizing_cycle +
            2;
    TEST_ASSERT_EQUAL(expected_cycles, tf.execute_until_state_change());
    tf.check_state(prop_state_t::await_firing);
}

// Test the case where we pressurize for the maximum 20 pressurizing cycles. Make sure that we still enter await_firing
void test_pressurize_to_firing()
{
    // There is no going from pressurizing into firing
    PropTestFixture tf;
    g_fake_pressure_cycle_count = 20;
    tf.set_state(prop_state_t::idle);
    tf.fixture_set_schedule(200, 400, 12, 800, PropState_Pressurizing::num_cycles_needed() + 1);
    tf.step();
    tf.check_state(prop_state_t::pressurizing);
    // g_fake_pressure_cycle_count causes us to execute 19 pressurizing cycles, enter the first cycle, and then exit pressurizing
    unsigned int expected_cycles =
            PropState_Pressurizing::num_cycles_needed() - PropState_Pressurizing::ctrl_cycles_per_firing_period + 2;
    // checking math
    unsigned int alias_cycles =
            (PropController::max_pressurizing_cycles - 1) * PropState_Pressurizing::ctrl_cycles_per_pressurizing_cycle +
            2;
    TEST_ASSERT_EQUAL(expected_cycles, alias_cycles);
    TEST_ASSERT_EQUAL(expected_cycles, tf.execute_until_state_change());
    tf.check_state(prop_state_t::await_firing);
    // There should be 1 firing cycles - 2 control cycles worth left then we should be in firing
    unsigned int expected_cycles_left = PropState_Pressurizing::ctrl_cycles_per_firing_period - 2;
    TEST_ASSERT_EQUAL(expected_cycles_left, tf.execute_until_state_change());
    tf.check_state(prop_state_t::firing);
}

// Test that when we are in pressurizing state, that Tank1 valve is opened and that Tank1 valve is closed when we
// enter await_firing
void test_pressurizing()
{
    PropTestFixture tf;
    tf.set_state(prop_state_t::idle);
    unsigned int cycles_until_fire = PropState_Pressurizing::num_cycles_needed() + 1;
    tf.fixture_set_schedule(700, 200, 200, 800, cycles_until_fire);
    tf.step(); // transitioned from idle -> pressurizing
    tf.check_state(prop_state_t::pressurizing);
    tf.step(); // first cycle of pressurizing
    TEST_ASSERT_TRUE(Tank1.is_valve_open(0)); // Tank1 valve is opened
    tf.execute_until_state_change();
    TEST_ASSERT_FALSE(Tank1.is_valve_open(0)); // Tank1 valve is not opened
    tf.check_state(prop_state_t::await_firing);
}

// Test the failure case where we are unable to reach threshold pressurize  in 20 cycles
void test_pressurize_fail()
{
    PropTestFixture tf;
    g_fake_pressure_cycle_count = 21;
    tf.set_state(prop_state_t::idle);
    tf.fixture_set_schedule(700, 200, 200, 800, PropState_Pressurizing::num_cycles_needed() + 1);
    tf.step();
    tf.check_state(prop_state_t::pressurizing);
    // finished last firing cycle + cycle where we closed the valve + cycle where we realize we fail
    unsigned int expected_cycles = PropState_Pressurizing::num_cycles_needed() + 2;
    TEST_ASSERT_EQUAL(expected_cycles, tf.execute_until_state_change());
    // TODO: sure we want to disable?
    tf.check_state(prop_state_t::disabled);
}

// Test that when we are in await_firing, all valves are closed, the the schedule is valid
void test_await_firing()
{
    PropTestFixture tf;
    tf.set_state(prop_state_t::idle);
    unsigned int cycles_until_fire = PropState_Pressurizing::num_cycles_needed() + 5;
    tf.fixture_set_schedule(700, 200, 200, 800, cycles_until_fire);
    TEST_ASSERT_EQUAL(1, tf.execute_until_state_change()); // await pressurizing
    tf.execute_until_state_change(); // pressurizing
    tf.execute_until_state_change(); // await_firing
    tf.check_state(prop_state_t::await_firing);
    // valves should be closed
    TEST_ASSERT_FALSE(Tank1.is_valve_open(0));
    TEST_ASSERT_FALSE(Tank2.is_valve_open(0));
    TEST_ASSERT_FALSE(Tank2.is_valve_open(1));
    TEST_ASSERT_FALSE(Tank2.is_valve_open(2));
    TEST_ASSERT_FALSE(Tank2.is_valve_open(3));
    // schedule should be valid
    TEST_ASSERT_TRUE(tf.prop_controller->validate_schedule());
}

// Test that PropulsionSystem is_firing() returns True iff we are in the firing state
void test_firing()
{
    PropTestFixture tf;
    tf.set_state(prop_state_t::idle);
    TEST_ASSERT_FALSE(PropulsionSystem.is_firing());
    unsigned int cycles_until_fire = PropState_Pressurizing::num_cycles_needed() + 5;
    tf.fixture_set_schedule(700, 200, 200, 800, cycles_until_fire);
    tf.step(cycles_until_fire);
    tf.check_state(prop_state_t::firing);
    TEST_ASSERT_TRUE(PropulsionSystem.is_firing())
    unsigned int expected_firing_cycles = (800 / PAN::control_cycle_time_ms) + 1; // roundup
    TEST_ASSERT_EQUAL(expected_firing_cycles, tf.execute_until_state_change());
    TEST_ASSERT_FALSE(PropulsionSystem.is_firing());
    // can't really check valve states unless we want to make a fake interval timer
}

// Test that if we order Prop to fire at PropState_Pressurizing::num_cycles_needed() + 1
// then Prop is definitely firing after this amount of cycles
void test_firing_to_idle()
{
    PropTestFixture tf;
    tf.set_state(prop_state_t::idle);
    unsigned int cycles_until_fire = PropState_Pressurizing::num_cycles_needed() + 1;
    tf.fixture_set_schedule(700, 200, 200, 800, cycles_until_fire);

    // [cycles_until_fire] from now, this had better be firing.
    tf.step(cycles_until_fire);
    tf.check_state(prop_state_t::firing);

    // Test that PropulsionSystem is firing while we are in the firing state
    // 800 is the biggest value
    unsigned int cycles_firing = (800 / PAN::control_cycle_time_ms) + 1; // round up
    for (size_t i = 0; i < cycles_firing; ++i) {
        tf.check_state(prop_state_t::firing);
        TEST_ASSERT_TRUE(PropulsionSystem.is_firing())
        tf.step();
    }
    // On the next control cycle, we should be back to idle
    TEST_ASSERT_FALSE(PropulsionSystem.is_firing())
    tf.check_state(prop_state_t::idle);
}

int test_prop_controller()
{
    // generated the following with:
    // cat test/test_fsw_prop_controller//test_prop_controller.cpp | grep "void test_" | sed 's/^void \(.*\)$/\1/' | sed 's/()/);/g'| sed -e 's/^/RUN_TEST(/'
    UNITY_BEGIN();
    RUN_TEST(test_initialization);
    RUN_TEST(test_disable);
    RUN_TEST(test_illegal_schedule);
    RUN_TEST(test_idle_to_pressurizing);
    RUN_TEST(test_idle_to_await_pressurize);
    RUN_TEST(test_await_pressurize_to_pressurize);
    RUN_TEST(test_pressurize_to_await_firing);
    RUN_TEST(test_pressurize_to_firing);
    RUN_TEST(test_pressurizing);
    RUN_TEST(test_pressurize_fail);
    RUN_TEST(test_await_firing);
    RUN_TEST(test_firing);
    RUN_TEST(test_firing_to_idle);
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
