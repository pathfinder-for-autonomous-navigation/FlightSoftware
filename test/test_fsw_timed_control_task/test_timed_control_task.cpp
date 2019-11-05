#include "../../src/FCCode/TimedControlTask.hpp"
#include "../../src/FCCode/ClockManager.hpp"
#include <unity.h>
#include <iostream>

class DummyTimedControlTask : public TimedControlTask<void> {
  public:
    DummyTimedControlTask(StateFieldRegistry &registry, const unsigned int offset) :
      TimedControlTask<void>(registry, offset) {}

    int i = 0;
    void execute() {
      i++;
    }
};

class TestFixture {
  public:
    StateFieldRegistry registry;
    std::unique_ptr<ClockManager> clock_manager;
    std::unique_ptr<DummyTimedControlTask> dummy_task_1;
    std::unique_ptr<DummyTimedControlTask> dummy_task_2;

    TestFixture() : registry() {
        const unsigned int control_cycle_size = 8000000;
        clock_manager = std::make_unique<ClockManager>(registry, control_cycle_size);

        constexpr unsigned int allocated_starts[2] = {2001, 6001};
        dummy_task_1 = std::make_unique<DummyTimedControlTask>(registry, allocated_starts[0]);
        dummy_task_2 = std::make_unique<DummyTimedControlTask>(registry, allocated_starts[1]);
    }

    systime_t execute() {
        clock_manager->execute();
        dummy_task_1->execute_on_time(clock_manager->get_control_cycle_start_time());
        systime_t ret_time = get_system_time();
        dummy_task_2->execute_on_time(clock_manager->get_control_cycle_start_time());
        return ret_time;
    }

    systime_t get_system_time() {
        return clock_manager->get_system_time();
    }

    unsigned int duration_to_us(const systime_duration_t& duration) {
        return clock_manager->duration_to_us(duration);
    }
};

void test_task_initialization() {
    TestFixture tf;
}

void test_task_execute() {
    TestFixture tf;
    systime_t t_start, t_end1, t_end2;
    unsigned int t_delta1, t_delta2;

    // Single-cycle test
    t_start = tf.get_system_time();
    t_end1 = tf.execute();
    t_end2 = tf.get_system_time();
    t_delta1 = tf.duration_to_us(t_end1 - t_start);
    t_delta2 = tf.duration_to_us(t_end2 - t_start);
    TEST_ASSERT_UINT32_WITHIN(10, 2000, t_delta1);
    TEST_ASSERT_UINT32_WITHIN(10, 6000, t_delta2);

    // Two-cycle test
    t_end1 = tf.execute();
    t_end2 = tf.get_system_time();
    t_delta1 = tf.duration_to_us(t_end1 - t_start);
    t_delta2 = tf.duration_to_us(t_end2 - t_start);
    TEST_ASSERT_UINT32_WITHIN(10, 10000, t_delta1);
    TEST_ASSERT_UINT32_WITHIN(10, 14000, t_delta2);

    // Stress test the timing guarantee for the control cycle
    // time over many cycles
    t_start = tf.get_system_time();
    for(int i = 0; i < 1000; i++) tf.execute();
    const systime_t t_end = tf.get_system_time();
    const unsigned int t_delta = tf.duration_to_us(t_end - t_start);
    TEST_ASSERT_UINT32_WITHIN(2000, 7998000, t_delta); // 2 ms drift over a period of 8 seconds. Pretty good!
}

int test_timed_control_task() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_task_execute);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_timed_control_task();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_timed_control_task();
}

void loop() {}
#endif
