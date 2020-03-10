#ifndef TEST_FAULT_HANDLERS_HPP_
#define TEST_FAULT_HANDLERS_HPP_

#include <unity.h>
#include "../StateFieldRegistryMock.hpp"

#define TEST_ASSERT_EQUAL_FAULT_RESPONSES(expected, actual) \
    TEST_ASSERT_EQUAL_MESSAGE(expected.recommend_safehold, actual.recommend_safehold, "Safehold inequality"); \
    TEST_ASSERT_EQUAL_MESSAGE(expected.recommend_standby, actual.recommend_standby, "Standby inequality"); \
    static_assert(true, "") // This is a hack to require a semicolon at the end of the macro

void test_fault_machine_mock();
void test_super_simple_fault_handlers();
void test_main_fault_handler();

// TODO
// void test_prop_overpressure_fault_handler();
// void test_quake_fault_handler();
// void test_piksi_fault_handler();
// void test_fault_handling_end_to_end();

#endif
