#include "test_fault_handlers.hpp"
#include "cartesian_product.hpp"
#include "../test_fsw_all_1/fsw_tests.hpp"

namespace fsw_test {
void test_fault_handlers() {
    UNITY_BEGIN();
    RUN_TEST(test_cartesian_product);
    RUN_TEST(test_fault_machine_mock);
    RUN_TEST(test_super_simple_fault_handlers);
    RUN_TEST(test_main_fault_handler);

    // TODO
    // RUN_TEST(test_prop_overpressure_fault_handler);
    RUN_TEST(test_quake_fault_handler);
    RUN_TEST(test_piksi_fault_handler);
    RUN_TEST(test_fault_handling_end_to_end);
    UNITY_END();
}
}

#ifndef COMBINE_TESTS
UNIT_TEST_RUNNER(fsw_tests::test_fault_handlers);
#endif
