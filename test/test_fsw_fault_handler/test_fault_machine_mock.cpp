#include "test_fault_handlers.hpp"
#include "../StateFieldRegistryMock.hpp"
#include "../shared/FaultHandlerMachineMock.hpp"

void test_fault_machine_mock() {
    StateFieldRegistryMock registry;
    FaultHandlerMachineMock fhmock(registry);

    fhmock.set(fault_response_t::none);
    TEST_ASSERT_EQUAL(fault_response_t::none, fhmock.execute());

    fhmock.set(fault_response_t::standby);
    TEST_ASSERT_EQUAL(fault_response_t::standby, fhmock.execute());

    fhmock.set(fault_response_t::safehold);
    TEST_ASSERT_EQUAL(fault_response_t::safehold, fhmock.execute());
}
