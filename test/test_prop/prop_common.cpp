
#ifndef PROP_COMMON_H_
#define PROP_COMMON_H_
#include <unity.h>
#include <Arduino.h>
#include <fsw/FCCode/Drivers/PropulsionSystem.hpp>
#define ASSERT_TRUE(x, msg){UNITY_TEST_ASSERT((x), __LINE__, msg);}
#define ASSERT_FALSE(x, msg){UNITY_TEST_ASSERT((!x), __LINE__, msg);}
/**
 * Helper Methods
 */
void check_all_valves_closed();
void check_tank2_schedule(const std::array<unsigned int, 4> &expected_schedule);
void check_tank2_valve_status(bool a, bool b, bool c, bool d);
void check_tank1_valve_status(bool a, bool b);

const std::array<unsigned int, 4> zero_schedule = {0, 0, 0, 0};

#endif
