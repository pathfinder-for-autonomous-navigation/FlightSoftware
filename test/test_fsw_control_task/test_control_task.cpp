#include "../StateFieldRegistryMock.hpp"
#include <ControlTask.hpp>
#include <unity.h>

class DummyControlTask : public ControlTask<void> {
  public:
    DummyControlTask(StateFieldRegistry& registry) :
      ControlTask<void>(registry),
      x_sr(0, 10, 10),
      x_f("dummy_x", x_sr)
    {
        y_fp = find_writable_field<unsigned int>("dummy_y", __FILE__, __LINE__);
        add_writable_field(x_f);
    }

    void execute() { 
      x_f.set(y_fp->get());
    }

  protected:
    std::shared_ptr<WritableStateField<unsigned int>> y_fp;
    
    Serializer<unsigned int> x_sr;
    WritableStateField<unsigned int> x_f;
};

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    std::shared_ptr<WritableStateField<unsigned int>> y_fp;

    std::unique_ptr<DummyControlTask> dummy_task;
    std::shared_ptr<WritableStateField<unsigned int>> x_fp;

    TestFixture() : registry() {
        y_fp = registry.create_writable_field<unsigned int>("dummy_y", 0, 10, 10);
        y_fp->set(3);

        dummy_task = std::make_unique<DummyControlTask>(registry);
        x_fp = std::static_pointer_cast<WritableStateField<unsigned int>>(registry.find_writable_field("dummy_x"));
    }
};

void test_task_initialization() {
    TestFixture tf;
    TEST_ASSERT_EQUAL(3, tf.y_fp->get());
}

void test_task_execute() {
    TestFixture tf;
    TEST_ASSERT_EQUAL(0, tf.x_fp->get());
    tf.y_fp->set(3);
    tf.dummy_task->execute();
    TEST_ASSERT_EQUAL(3, tf.x_fp->get());
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
