#include "fsw_tests.hpp"
#include "../custom_assertions.hpp"

namespace fsw_test {
int test_fsw_all_1() {
    UNITY_BEGIN();
    RUN_TEST(fsw_test::test_adcs_commander);
    RUN_TEST(fsw_test::test_adcs_box_monitor);
    RUN_TEST(fsw_test::test_attitude_computer);
    RUN_TEST(fsw_test::test_bitstream);
    RUN_TEST(fsw_test::test_control_task);
    RUN_TEST(fsw_test::test_dcdc);
    RUN_TEST(fsw_test::test_docking);
    RUN_TEST(fsw_test::test_downlink_producer);
    RUN_TEST(fsw_test::test_eeprom);
    RUN_TEST(fsw_test::test_events);
    RUN_TEST(fsw_test::test_fault);
    RUN_TEST(fsw_test::test_fault_handlers);
    RUN_TEST(fsw_test::test_gomspace);
    RUN_TEST(fsw_test::test_mission_manager);
    RUN_TEST(fsw_test::test_orbit_control_task);
    RUN_TEST(fsw_test::test_piksi_control_task);
    RUN_TEST(fsw_test::test_prop_controller);
    RUN_TEST(fsw_test::test_prop_fault_response);
    RUN_TEST(fsw_test::test_quake_manager);
    RUN_TEST(fsw_test::test_timed_control_task);
    RUN_TEST(fsw_test::test_uplink_consumer);
    return UNITY_END();
}
}

#ifdef COMBINE_TESTS
UNIT_TEST_RUNNER(fsw_test::test_fsw_all_1);
#endif
