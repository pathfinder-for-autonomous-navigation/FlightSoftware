#include "../StateFieldRegistryMock.hpp"

#include <adcs/constants.hpp>
#include <fsw/FCCode/AttitudeEstimator.hpp>

#include <lin/core.hpp>

#include "../custom_assertions.hpp"

class TestFixture {
    public:
        StateFieldRegistryMock registry;

        // pointers to input statefields
        std::shared_ptr<ReadableStateField<double>> orbit_time_fp;
        std::shared_ptr<ReadableStateField<lin::Vector3d>> pos_vec_ecef_fp;
        std::shared_ptr<ReadableStateField<lin::Vector3f>> ssa_vec_rd_fp;
        std::shared_ptr<ReadableStateField<lin::Vector3f>> mag1_vec_fp;
        std::shared_ptr<ReadableStateField<lin::Vector3f>> mag2_vec_fp;
        std::shared_ptr<ReadableStateField<lin::Vector3f>> gyr_vec_fp;

        // pointers to output statefields for easy access
        ReadableStateField<lin::Vector4f>* q_body_eci_fp;
        ReadableStateField<lin::Vector3f>* w_body_fp;
        ReadableStateField<float>* fro_P_fp;

        std::unique_ptr<AttitudeEstimator> attitude_estimator;

        // Create a TestFixture instance of AttitudeEstimator with pointers to statefields
        TestFixture() : registry(){
                //create input statefields
                orbit_time_fp = registry.create_readable_field<double>("orbit.time",0.0,18'446'744'073'709'551'616.0,64);
                pos_vec_ecef_fp = registry.create_readable_lin_vector_field<double>("orbit.pos",0.0L,1000000.0L,64*3);
                ssa_vec_rd_fp = registry.create_readable_lin_vector_field<float>("adcs_monitor.ssa_vec",-1.0,1.0,32*3),
                mag1_vec_fp = registry.create_readable_lin_vector_field<float>("adcs_monitor.mag1_vec",-16e-4,16e4,32*3),
                mag2_vec_fp = registry.create_readable_lin_vector_field<float>("adcs_monitor.mag2_vec",-16e-4,16e4,32*3),
                gyr_vec_fp = registry.create_readable_lin_vector_field<float>("adcs_monitor.gyr_vec",adcs::imu::min_rd_omega, adcs::imu::max_rd_omega, 16*3),

                attitude_estimator = std::make_unique<AttitudeEstimator>(registry, 0);  

                // initialize pointers to statefields
                q_body_eci_fp = registry.find_readable_field_t<lin::Vector4f>("attitude_estimator.q_body_eci");
                w_body_fp = registry.find_readable_field_t<lin::Vector3f>("attitude_estimator.w_body");
                fro_P_fp = registry.find_readable_field_t<float>("attitude_estimator.fro_P");
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

PAN_TEST(test_control_task)
