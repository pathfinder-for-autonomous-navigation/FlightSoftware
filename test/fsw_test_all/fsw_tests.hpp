#ifndef FSW_TESTS_HPP
#define FSW_TESTS_HPP

namespace fsw_test {
    void test_adcs_commander();
    void test_adcs_box_monitor();
    void test_attitude_computer();
    void test_bitstream();
    void test_control_task();
    void test_dcdc();
    void test_docking();
    void test_downlink_producer();
    void test_eeprom();
    void test_events();
    void test_fault();
    void test_fault_handlers();
    void test_gomspace();
    void test_mission_manager();
    void test_orbit_control_task();
    void test_piksi_control_task();
    void test_prop_controller();
    void test_prop_fault_response();
    void test_quake_manager();
    void test_timed_control_task();
    void test_uplink_consumer();
}

#endif
