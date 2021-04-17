#include "TimedControlTask.hpp"
#include <common/Fault.hpp>

sys_time_t TimedControlTaskBase::control_task_end_time;
unsigned int TimedControlTaskBase::control_cycle_count = 0;
