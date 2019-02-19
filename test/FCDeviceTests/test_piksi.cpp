#include <HardwareSerial.h>
#include <Piksi/Piksi.hpp>
#include <unity.h>
#include <unity_test/unity_fixture.h>

using namespace Devices;
Piksi piksi(Serial4);
bool piksi_setup;
bool data_in_buffer;

TEST_GROUP(PiksiTests);

TEST_SETUP(PiksiTests) {
    if (!piksi_setup) {
        Serial4.begin(9600);
        piksi.setup();
        piksi_setup = true;
    }
    if(!piksi.is_functional()) TEST_IGNORE_MESSAGE("Piksi is not connected/not functional");
}

TEST_TEAR_DOWN(PiksiTests) {}

TEST(PiksiTests, test_process_buffer) {
    
}

TEST(PiksiTests, test_get_gps_time) {

}

TEST(PiksiTests, test_get_pos_ecef_data) {
    
}

TEST(PiksiTests, test_get_baseline_ecef_data) {
    
}

TEST(PiksiTests, test_get_vel_ecef_data) {
    
}

TEST(PiksiTests, test_get_base_pos_ecef_data) {
    
}

TEST(PiksiTests, test_get_iar) {
    
}

TEST(PiksiTests, test_get_heartbeat_data) {
    
}

TEST(PiksiTests, test_get_uarta_data) {
    
}

TEST(PiksiTests, test_get_uartb_data) {
    
}

TEST(PiksiTests, test_write_default_settings) {
    
}

TEST(PiksiTests, test_piksi_reset) {
    
}

TEST_GROUP_RUNNER(PiksiTests) {
    RUN_TEST_CASE(PiksiTests, test_process_buffer);
    RUN_TEST_CASE(PiksiTests, test_get_gps_time);
    RUN_TEST_CASE(PiksiTests, test_get_pos_ecef_data);
    RUN_TEST_CASE(PiksiTests, test_get_baseline_ecef_data);
    RUN_TEST_CASE(PiksiTests, test_get_base_pos_ecef_data);
    RUN_TEST_CASE(PiksiTests, test_get_iar);
    RUN_TEST_CASE(PiksiTests, test_get_heartbeat_data);
    RUN_TEST_CASE(PiksiTests, test_get_uarta_data);
    RUN_TEST_CASE(PiksiTests, test_get_uartb_data);
    RUN_TEST_CASE(PiksiTests, test_write_default_settings);
    RUN_TEST_CASE(PiksiTests, test_piksi_reset);
}