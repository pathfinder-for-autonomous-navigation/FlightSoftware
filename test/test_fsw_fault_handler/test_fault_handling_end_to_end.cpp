#include "test_fault_handlers.hpp"
#include <fsw/FCCode/MainFaultHandler.hpp>
#include <algorithm>
#include <numeric>
#include "cartesian_product.hpp"

void test_adcs_faults() {
    
}

void test_fault_handling_end_to_end() {
    RUN_TEST(test_adcs_faults);
}
