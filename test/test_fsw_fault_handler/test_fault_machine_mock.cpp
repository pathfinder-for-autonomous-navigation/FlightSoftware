#include "test_fault_handlers.hpp"
#include "../StateFieldRegistryMock.hpp"
#include "../FaultHandlerMachineMock.hpp"

void test_fault_machine_mock() {
    StateFieldRegistryMock registry;
    FaultHandlerMachineMock fhmock(registry);

    fhmock.set(no_fault_response);
    TEST_ASSERT_EQUAL_FAULT_RESPONSES(no_fault_response, fhmock.execute());

    fhmock.set(standby_fault_response);
    TEST_ASSERT_EQUAL_FAULT_RESPONSES(standby_fault_response, fhmock.execute());

    fhmock.set(safehold_fault_response);
    TEST_ASSERT_EQUAL_FAULT_RESPONSES(safehold_fault_response, fhmock.execute());
}
