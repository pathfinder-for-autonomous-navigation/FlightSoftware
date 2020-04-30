#ifndef TEST_FAULT_HANDLERS_HPP_
#define TEST_FAULT_HANDLERS_HPP_

#include <unity.h>
#undef isnan
#undef isinf
#include "../StateFieldRegistryMock.hpp"

void test_fault_machine_mock();
void test_super_simple_fault_handlers();
void test_main_fault_handler();

// TODO
// void test_prop_overpressure_fault_handler();
// void test_quake_fault_handler();
// void test_piksi_fault_handler();
// void test_fault_handling_end_to_end();

#endif
