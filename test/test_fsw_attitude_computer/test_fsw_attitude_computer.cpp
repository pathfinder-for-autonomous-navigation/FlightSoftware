#include "../StateFieldRegistryMock.hpp"
#include "../src/FCCode/AttitudeComputer.hpp"
#include "../src/FCCode/adcs_state_t.enum"

#include <unity.h>

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    // Input state fields to attitude computer
    std::shared_ptr<ReadableStateField<f_vector_t>> ssa_vec_fp;
    std::shared_ptr<ReadableStateField<gps_time_t>> time_fp;
    std::shared_ptr<ReadableStateField<d_vector_t>> pos_fp;
    std::shared_ptr<ReadableStateField<d_vector_t>> pos_baseline_fp;

    // Attitude computer
    std::unique_ptr<AttitudeComputer> attitude_computer;
    
    // Output state fields
    const WritableStateField<unsigned char>* adcs_state_fp;
    const WritableStateField<f_vector_t>* adcs_vec1_current_fp;
    const WritableStateField<f_vector_t>* adcs_vec1_desired_fp;
    const WritableStateField<f_vector_t>* adcs_vec2_current_fp;
    const WritableStateField<f_vector_t>* adcs_vec2_desired_fp;

    TestFixture() : registry() {
        ssa_vec_fp = registry.create_readable_vector_field<float>("adcs_monitor.ssa_vec", 0, 1, 100);
        time_fp = registry.create_readable_field<gps_time_t>("piksi.time");
        pos_fp = registry.create_readable_vector_field<double>("piksi.pos", 0, 100000, 100);
        pos_baseline_fp = registry.create_readable_vector_field<double>("piksi.baseline_pos", 0, 100000, 100);

        attitude_computer = std::make_unique<AttitudeComputer>(registry, 0);

        // Check that attitude computer creates its expected fields
        adcs_state_fp = registry.find_writable_field_t<unsigned char>("adcs.state");
        adcs_vec1_current_fp = registry.find_writable_field_t<f_vector_t>("adcs.control.vec1.current");
        adcs_vec1_desired_fp = registry.find_writable_field_t<f_vector_t>("adcs.control.vec1.desired");
        adcs_vec2_current_fp = registry.find_writable_field_t<f_vector_t>("adcs.control.vec2.current");
        adcs_vec2_desired_fp = registry.find_writable_field_t<f_vector_t>("adcs.control.vec2.desired");
    }
};

void test_valid_initialization() {
    TestFixture tf;
}

int test_attitude_computer() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_attitude_computer();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_attitude_computer();
}

void loop() {}
#endif
