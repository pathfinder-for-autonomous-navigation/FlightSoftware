#include "../StateFieldRegistryMock.hpp"

#include <fsw/FCCode/AttitudeEstimator.hpp>

#include <unity.h>

class TestFixture {
    public:
        StateFieldRegistryMock registry;

        // pointers to input statefields
        std::shared_ptr<ReadableStateField<gps_time_t>> piksi_time_fp;
        std::shared_ptr<ReadableStateField<d_vector_t>> pos_vec_ecef_fp;
        std::shared_ptr<ReadableStateField<f_vector_t>> ssa_vec_rd_fp;
        std::shared_ptr<ReadableStateField<f_vector_t>> mag_vec_fp;

        // pointers to output statefields for easy access
        ReadableStateField<f_quat_t>* q_body_eci_fp;
        ReadableStateField<f_vector_t>* w_body_fp;
        InternalStateField<lin::Vector3f>* h_body_fp;
        WritableStateField<bool>* adcs_paired_fp;

        std::unique_ptr<AttitudeEstimator> attitude_estimator;

        // Create a TestFixture instance of AttitudeEstimator with pointers to statefields
        TestFixture() : registry(){

                //create input statefields
                piksi_time_fp = registry.create_readable_field<gps_time_t>("piksi.time");
                pos_vec_ecef_fp = registry.create_readable_vector_field<double>("piksi.pos",0.0L,1000000.0L,64*3);
                ssa_vec_rd_fp = registry.create_readable_vector_field<float>("adcs_monitor.ssa_vec",-1.0,1.0,32*3),
                mag_vec_fp = registry.create_readable_vector_field<float>("adcs_monitor.mag_vec",-16e-4,16e4,32*3),

                attitude_estimator = std::make_unique<AttitudeEstimator>(registry, 0);  

                // initialize pointers to statefields
                q_body_eci_fp = registry.find_readable_field_t<f_quat_t>("attitude_estimator.q_body_eci");
                w_body_fp = registry.find_readable_field_t<f_vector_t>("attitude_estimator.w_body");
                h_body_fp = registry.find_internal_field_t<lin::Vector3f>("attitude_estimator.h_body");
                adcs_paired_fp = registry.find_writable_field_t<bool>("adcs.paired");
        }
};

void test_task_initialization()
{
        TestFixture tf;
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