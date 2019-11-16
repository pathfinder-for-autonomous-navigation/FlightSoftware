#include <TimedControlTask.hpp>
#include "../../src/FCCode/ClockManager.hpp"
#include <unity.h>
#ifdef DESKTOP
    #include <iostream>
#else
    #include <Arduino.h>
#endif

/**
 * @brief Dummy control task that just increments a member variable.
 */
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

    static constexpr unsigned int control_cycle_ms = 8000;
    #ifdef DESKTOP
        // Control cycle time must be specified in nanoseconds on desktop
        static constexpr unsigned int control_cycle_size = control_cycle_ms * 1000;
    #else
        // Control cycle time must be specified in microseconds on Teensy
        static constexpr unsigned int control_cycle_size = control_cycle_ms;
    #endif

    /**
     * @brief Construct a new Test Fixture.
     * 
     * It initializes the clock manager and two dummy tasks. Dummy task 1 
     * starts 2 ms after the beginning of the control cycle; dummy task 2
     * starts 6 ms after the beginning of the cycle.
     */
    TestFixture() : registry() {
        clock_manager = std::make_unique<ClockManager>(registry, control_cycle_size);

        constexpr unsigned int allocated_starts[2] = {2001, 6001};
        dummy_task_1 = std::make_unique<DummyTimedControlTask>(registry, allocated_starts[0]);
        dummy_task_2 = std::make_unique<DummyTimedControlTask>(registry, allocated_starts[1]);
    }

    /**
     * @brief Run the control loop, consisting of the Clock Manager and two dummy tasks.
     * 
     * @return sys_time_t The system time at which the first task completed.
     */
    sys_time_t execute() {
        clock_manager->execute();
        dummy_task_1->execute_on_time(clock_manager->get_control_cycle_start_time());
        sys_time_t ret_time = get_system_time();
        dummy_task_2->execute_on_time(clock_manager->get_control_cycle_start_time());
        return ret_time;
    }

    /**
     * @brief Get the current system time from the clock manager.
     * 
     * @return sys_time_t 
     */
    sys_time_t get_system_time() {
        return clock_manager->get_system_time();
    }

    /**
     * @brief Convert system time intervals to an integer number of microseconds.
     * 
     * @param duration System time interval.
     * @return unsigned int 
     */
    unsigned int duration_to_us(const systime_duration_t& duration) {
        return clock_manager->duration_to_us(duration);
    }
};

// Create storage for control cycle size in Test Fixture
constexpr unsigned int TestFixture::control_cycle_ms;
constexpr unsigned int TestFixture::control_cycle_size;

void test_task_initialization() {
    TestFixture tf;
}

void test_task_execute() {
    #ifdef CI
    TEST_IGNORE_MESSAGE("This test doesn't work on virtual machines.");
    #endif

    TestFixture tf;
    sys_time_t t_start, t_end1, t_end2;
    unsigned int t_delta1, t_delta2;

    // Test that a single cycle of the control loop keeps timing OK.
    t_start = tf.get_system_time();
    t_end1 = tf.execute();
    t_end2 = tf.get_system_time();
    t_delta1 = tf.duration_to_us(t_end1 - t_start);
    t_delta2 = tf.duration_to_us(t_end2 - t_start);
    TEST_ASSERT_UINT32_WITHIN(20, 2000, t_delta1);
    TEST_ASSERT_UINT32_WITHIN(20, 6000, t_delta2);

    // Test that the timing is OK within two executions of the control loop.
    t_end1 = tf.execute();
    t_end2 = tf.get_system_time();
    t_delta1 = tf.duration_to_us(t_end1 - t_start);
    t_delta2 = tf.duration_to_us(t_end2 - t_start);
    TEST_ASSERT_UINT32_WITHIN(20, 10000, t_delta1);
    TEST_ASSERT_UINT32_WITHIN(20, 14000, t_delta2);

    // Stress test the timing guarantee for the control cycle time over many cycles
    t_start = tf.get_system_time();
    for(int i = 0; i < 1000; i++) tf.execute();
    const sys_time_t t_end = tf.get_system_time();
    const unsigned int t_delta = tf.duration_to_us(t_end - t_start);

    // We subtract 2000 us since the second task will finish early.
    constexpr unsigned long expected_duration = TestFixture::control_cycle_ms * 1000 - 2000;

    // 4 ms drift over a period of 8 seconds (0.05% clock lag.) Pretty good!
    TEST_ASSERT_UINT64_WITHIN(4000, expected_duration, t_delta);
}

int test_timed_control_task() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_task_execute);
    return UNITY_END();
}

const char* warning = "PLEASE BE PATIENT. This unit test takes a while "
                      "because it runs through 1000 control cycles.";
#ifdef DESKTOP
int main() {
    std::cout << warning << std::endl;
    return test_timed_control_task();
}
#else
void setup() {
    delay(2000);
    Serial.begin(9600);
    Serial.println(warning);
    test_timed_control_task();
}

void loop() {}
#endif
