#include "../StateFieldRegistryMock.hpp"
#include <fsw/FCCode/OrbitController.hpp>
#include <fsw/FCCode/prop_planner_state_t.enum>
#include "../custom_assertions.hpp"

class TestFixture {
    public:
        StateFieldRegistryMock registry;

        // Inputs to orbit controller
        std::shared_ptr<ReadableStateField<double>> time_fp;
        std::shared_ptr<ReadableStateField<lin::Vector3d>> pos_fp;
        std::shared_ptr<ReadableStateField<lin::Vector3d>> vel_fp;
        std::shared_ptr<ReadableStateField<lin::Vector3d>> baseline_pos_fp;
        std::shared_ptr<ReadableStateField<lin::Vector3d>> baseline_vel_fp;

        std::unique_ptr<OrbitController> orbit_controller;

        // Outputs of orbit controller
        WritableStateField<unsigned int>* sched_valve1_fp;
        WritableStateField<unsigned int>* sched_valve2_fp;
        WritableStateField<unsigned int>* sched_valve3_fp;
        WritableStateField<unsigned int>* sched_valve4_fp;

        // Create a TestFixture instance of PiksiController with pointers to statefields
        TestFixture() : registry() {
                time_fp = registry.create_readable_field<double>("orbit.time", 0, 0, 1);
                pos_fp = registry.create_readable_lin_vector_field<double>("orbit.pos", 0, 0, 100);
                vel_fp = registry.create_readable_lin_vector_field<double>("orbit.vel", 0, 0, 100);
                baseline_pos_fp = registry.create_readable_lin_vector_field<double>("orbit.baseline_pos", 0, 0, 100);
                baseline_vel_fp = registry.create_readable_lin_vector_field<double>("orbit.baseline_vel", 0, 0, 100);

                orbit_controller = std::make_unique<OrbitController>(registry, 0);  

                sched_valve1_fp = registry.find_writable_field_t<unsigned int>("orbit.control.valve1");
                sched_valve2_fp = registry.find_writable_field_t<unsigned int>("orbit.control.valve2");
                sched_valve3_fp = registry.find_writable_field_t<unsigned int>("orbit.control.valve3");
                sched_valve4_fp = registry.find_writable_field_t<unsigned int>("orbit.control.valve4");
        }
};

void test_task_initialization()
{
        TestFixture tf;
        TEST_ASSERT_NOT_NULL(tf.sched_valve1_fp);
        TEST_ASSERT_NOT_NULL(tf.sched_valve2_fp);
        TEST_ASSERT_NOT_NULL(tf.sched_valve3_fp);
        TEST_ASSERT_NOT_NULL(tf.sched_valve4_fp);
}

int test_control_task()
{
        UNITY_BEGIN();
        RUN_TEST(test_task_initialization);
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
