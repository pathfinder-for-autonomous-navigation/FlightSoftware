#include "../StateFieldRegistryMock.hpp"
#include <fsw/FCCode/OrbitController.hpp>
#include <fsw/FCCode/AttitudeEstimator.hpp>
#include <fsw/FCCode/prop_planner_state_t.enum>
#include "../custom_assertions.hpp"
#include <iostream>

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
        std::unique_ptr<AttitudeEstimator> attitude_controller;

        // Outputs of orbit controller
        std::shared_ptr<ReadableStateField<unsigned char>> prop_cycles_until_firing_fp;
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
                prop_cycles_until_firing_fp = registry.create_readable_field<unsigned char>("prop.cycles_until_firing", 0);

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

void test_task_time_till_node(){
        TestFixture tf;

        // Set parameters
        double theta = 0;
        lin::Vector3d r = {1,0,0};
        lin::Vector3d v = {0,1,0};

        // Calculate expected time (next firing node is pi/3), omega = 1;
        double firing_node = gnc::constant::pi/3;
        double time = firing_node;

        TEST_ASSERT_EQUAL(time, tf.orbit_controller->time_till_node(theta, r, v));
}

void test_task_calculate_impulse(){
        TestFixture tf;

        // Put random values for parameters.
        double t = 10;
        lin::Vector3d r = {1,0,0};
        lin::Vector3d v = {0,1,0};
        lin::Vector3d dr = {0,1,0};
        lin::Vector3d dv = {-1,0,0};

        // Check that the orbit controller structs are initially null
        PAN_TEST_ASSERT_EQUAL_DOUBLE_LIN_VEC(lin::nans<lin::Vector3d>(), tf.orbit_controller->actuation.J_ecef, 1e-10);

        // Check that orbit controller actually returns a value (actuation is not NaN)
        tf.orbit_controller->calculate_impulse(t, r, v, dr, dv);

        PAN_TEST_ASSERT_NOT_NAN_DOUBLE_LIN_VEC(tf.orbit_controller->actuation.J_ecef);
}

void test_task_schedule_valves_helper(lin::Vector3d J_body){
        TestFixture tf;

        tf.orbit_controller->schedule_valves(J_body);

        // Unit vectors giving the directions of the thrusters in the satellite's body frame
        lin::Vector3d thruster1 = { 0.6534, -0.3822, -0.6534};
        lin::Vector3d thruster2 = { 0.5391,  0.6472,  0.5391};
        lin::Vector3d thruster3 = {-0.6534, -0.3822,  0.6534};
        lin::Vector3d thruster4 = {-0.5391,  0.6472, -0.5391};
        lin::Matrix3x4d thrust_matrix = {
                thruster1(0), thruster2(0), thruster3(0), thruster4(0),
                thruster1(1), thruster2(1), thruster3(1), thruster4(1),
                thruster1(2), thruster2(2), thruster3(2), thruster4(2),
        };

        // Find the calculated impulse for each thruster
        unsigned int t1 = tf.sched_valve1_fp->get();
        double j1 = (t1-7.0092e-05) / (0.024119*1000);
        unsigned int t2 = tf.orbit_controller->sched_valve2_f.get();
        double j2 = (t2-7.0092e-05) / (0.024119*1000);
        unsigned int t3 = tf.orbit_controller->sched_valve3_f.get();
        double j3 = (t3-7.0092e-05) / (0.024119*1000);
        unsigned int t4 = tf.orbit_controller->sched_valve4_f.get();
        double j4 = (t4-7.0092e-05) / (0.024119*1000);
        lin::Vector4d impulses = {j1, j2, j3, j4};

        // Check that they add up to the desired impulse vector J_ecef
        lin::Vector3d net_impulse = thrust_matrix*impulses;
        
        PAN_TEST_ASSERT_EQUAL_DOUBLE_LIN_VEC(J_body, net_impulse, 0.1);
}

// Check that we can get the right linear combination of impulses
void test_task_schedule_valves(){

        test_task_schedule_valves_helper({1,3,7});
        test_task_schedule_valves_helper({-3,-2,4});
        test_task_schedule_valves_helper({-1,10,-4});
        test_task_schedule_valves_helper({7,88,-7});
        test_task_schedule_valves_helper({7,8,-100});
        test_task_schedule_valves_helper({187,8,-50});
        test_task_schedule_valves_helper({29,83,-50});
        test_task_schedule_valves_helper({-27,-8,-50});
        
}

int test_control_task()
{
        UNITY_BEGIN();
        RUN_TEST(test_task_initialization);
        RUN_TEST(test_task_time_till_node);
        RUN_TEST(test_task_calculate_impulse);
        RUN_TEST(test_task_schedule_valves);
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
