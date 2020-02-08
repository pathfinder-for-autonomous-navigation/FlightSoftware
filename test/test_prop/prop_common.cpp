#include "prop_common.h"
extern Devices::PropulsionSystem prop_system;
using namespace Devices;
// Make sure all pins low
void check_all_valves_closed()
{
    for (size_t i = 0; i < 2; ++i)
        TEST_ASSERT_EQUAL(LOW, Tank1.is_valve_open(i));
    for (size_t i = 0; i < 4; ++i)
        TEST_ASSERT_EQUAL(LOW, Tank2.is_valve_open(i));
}

// Make sure that the current schedule matches the provided schedule
void check_tank2_schedule(const std::array<unsigned int, 4> &expected_schedule)
{
    for (size_t i = 0; i < 4; ++i)
        TEST_ASSERT_EQUAL(expected_schedule[i], Tank2.get_schedule_at(i));
}


void check_tank2_valve_status(bool a, bool b, bool c, bool d)
{
    TEST_ASSERT_EQUAL_MESSAGE(a, Tank2.is_valve_open(0), "tank2 valve 0");
    TEST_ASSERT_EQUAL_MESSAGE(b, Tank2.is_valve_open(1), "tank2 valve 1");
    TEST_ASSERT_EQUAL_MESSAGE(c, Tank2.is_valve_open(2), "tank2 valve 2");
    TEST_ASSERT_EQUAL_MESSAGE(d, Tank2.is_valve_open(3), "tank2 valve 3");
}

void check_tank1_valve_status(bool a, bool b)
{
    TEST_ASSERT_EQUAL_MESSAGE(a, Tank1.is_valve_open(0), "tank1 valve 0");
    TEST_ASSERT_EQUAL_MESSAGE(b, Tank1.is_valve_open(1), "tank1 valve 1");
}