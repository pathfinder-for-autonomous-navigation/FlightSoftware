#include "../../src/FCCode/TimedControlTask.hpp"
#include "../../src/FCCode/ClockManager.hpp"
#include <unity.h>

class DummyTimedControlTask : public TimedControlTask<unsigned int> {
  public:
    DummyTimedControlTask(StateFieldRegistry &registry, const unsigned int offset) :
      TimedControlTask<unsigned int>(registry, offset) {}

    int i = 0;
    unsigned int execute() {
      i++;
      return i;
    }
};

class TestFixture {
  public:
    StateFieldRegistry registry;
    std::unique_ptr<ClockManager> clock_manager;
    std::unique_ptr<DummyTimedControlTask> dummy_task_1;
    std::unique_ptr<DummyTimedControlTask> dummy_task_2;

    TestFixture() : registry() {
        const unsigned int control_cycle_size = 8000;
        clock_manager = std::make_unique<ClockManager>(registry, control_cycle_size);

        constexpr unsigned int allocated_times[2] = {2000, 4000};
        dummy_task_1 = std::make_unique<DummyTimedControlTask>(registry, allocated_times[0]);
        dummy_task_2 = std::make_unique<DummyTimedControlTask>(registry, allocated_times[1]);
    }

    void execute() {
        clock_manager->execute();
        dummy_task_1->execute_on_time(clock_manager->get_control_cycle_start_time());
        dummy_task_2->execute_on_time(clock_manager->get_control_cycle_start_time());
    }

    systime_duration_t get_time_delta(const systime_t& t1, const systime_t& t2) {
        return clock_manager->get_time_delta(t1, t2);
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
    systime_t t_start, t_end;
    unsigned int t_delta;

    // Single-cycle test
    t_start = tf.get_system_time();
    tf.execute();
    t_end = tf.get_system_time();
    t_delta = tf.duration_to_us(tf.get_time_delta(t_start, t_end));
    TEST_ASSERT_GREATER_OR_EQUAL(6000, t_delta);
    TEST_ASSERT_LESS_OR_EQUAL(6020, t_delta);

    // Two-cycle test
    systime_t t_original_start = t_start;
    t_start = tf.get_system_time();
    tf.execute();
    t_end = tf.get_system_time();
    t_delta = tf.duration_to_us(tf.get_time_delta(t_start, t_end));
    TEST_ASSERT_GREATER_OR_EQUAL(8000, t_delta);
    TEST_ASSERT_LESS_OR_EQUAL(8100, t_delta);
    unsigned int t_total_delta = tf.duration_to_us(tf.get_time_delta(t_original_start, t_end));
    TEST_ASSERT_GREATER_OR_EQUAL(16000, t_total_delta);
    TEST_ASSERT_LESS_OR_EQUAL(16020, t_total_delta);

    // Stress test many cycles
    t_start = tf.get_system_time();
    for(int i = 0; i < 100; i++) {
        tf.execute();
    }
    t_end = tf.get_system_time();
    t_delta = tf.duration_to_us(tf.get_time_delta(t_start, t_end));
    TEST_ASSERT_GREATER_OR_EQUAL(80000, t_delta);
    TEST_ASSERT_LESS_OR_EQUAL(80020, t_delta);
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
