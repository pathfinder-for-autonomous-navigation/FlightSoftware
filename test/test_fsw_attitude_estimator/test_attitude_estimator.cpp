#include "../StateFieldRegistryMock.hpp"

#include "../../src/FCCode/AttitudeEstimator.hpp"
#include <unity.h>

class TestFixture {
    public:
        StateFieldRegistryMock registry;

        // pointers to statefields for easy access
        ReadableStateField<f_quat_t>* q_body_eci_fp;
        ReadableStateField<f_vector_t>* w_body_fp;

        std::unique_ptr<AttitudeEstimator> attitude_estimator;

        // Create a TestFixture instance of AttitudeEstimator with pointers to statefields
        TestFixture() : registry(){

                attitude_estimator = std::make_unique<AttitudeEstimator>(registry, 0);  

                // initialize pointers to statefields
                q_body_eci_fp = registry.find_readable_field_t<f_quat_t>("attitude_estimator.q_body_eci");
                w_body_fp = registry.find_readable_field_t<f_vector_t>("attitude_estimator.w_body");

                assert(q_body_eci_fp);
                assert(w_body_fp);
        
        }
};

void test_task_initialization()
{
        TestFixture tf;
}

void test_execute(){

}

int test_control_task()
{
        UNITY_BEGIN();
        RUN_TEST(test_task_initialization);
        RUN_TEST(test_execute);
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