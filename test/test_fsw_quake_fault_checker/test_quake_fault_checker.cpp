#include <unity.h>
#include "../StateFieldRegistryMock.hpp"
#include <fsw/FCCode/QuakeFaultChecker.hpp>

class TestFixture {
  public:
    StateFieldRegistryMock registry;

    std::shared_ptr<InternalStateField<unsigned char>> radio_state_fp;
    std::shared_ptr<InternalStateField<unsigned int>> last_checkin_cycle_fp;
    std::shared_ptr<WritableStateField<bool>> radio_power_cycle_fp;

    std::unique_ptr<QuakeFaultChecker> qfc;

    TestFixture() {
        radio_state_fp = registry.create_internal_field<unsigned char>("radio.state");
        last_checkin_cycle_fp = registry.create_internal_field<unsigned int>("radio.last_comms_ccno");
        radio_power_cycle_fp = registry.create_writable_field<bool>("gomspace.power_cycle_output1_cmd");

        qfc = std::make_unique<QuakeFaultChecker>(registry);
    }

    void step() {
        qfc->execute();
    }
};

/**
 * @brief Unit test the mocked fault checker utility so that we know
 * it'll work for Mission Manager.
 */
void test_qfcmock() {
    StateFieldRegistryMock r;
    QuakeFaultCheckerMock qfc(r);
    qfc.set(mission_state_t::follower);
    TEST_ASSERT_EQUAL(mission_state_t::follower, qfc.get());
    TEST_ASSERT_EQUAL(mission_state_t::follower, qfc.execute());
}

void test_qfc_initialization() {
    TestFixture tf;
}

int test_mission_manager() {
    UNITY_BEGIN();
    RUN_TEST(test_qfcmock);
    RUN_TEST(test_qfc_initialization);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_mission_manager();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_mission_manager();
}

void loop() {}
#endif
