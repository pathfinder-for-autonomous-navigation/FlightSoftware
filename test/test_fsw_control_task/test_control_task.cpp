#include <fsw/FCCode/ControlTask.hpp>

#include <unity.h>

class DummyControlTask : public ControlTask<void> {
  public:
    DummyControlTask(StateFieldRegistry& registry) : ControlTask<void>(registry) {}
    void execute() { 
      DummyControlTask::x = 3;
    }
    int x = 2;
};

void test_task_initialization() {
    StateFieldRegistry registry;
    DummyControlTask task(registry);
}

void test_task_execute() {
    StateFieldRegistry registry;
    DummyControlTask task(registry);
    TEST_ASSERT_EQUAL(2, task.x);
    task.execute();
    TEST_ASSERT_EQUAL(3, task.x);
}

int test_control_task() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_task_execute);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_control_task();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_control_task();
}

void loop() {}
#endif
