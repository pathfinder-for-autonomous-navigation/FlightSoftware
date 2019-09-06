#include "MainControlLoopTask.hpp"
#include "DebugTask.hpp"

#include <Arduino.h>

MainControlLoopTask::MainControlLoopTask(StateFieldRegistry& registry)
    : ControlTask<void>("fcp", registry), debug_task(registry)
{
    Serializer<signed int> temperature_serializer(-40, 125, SerializerConstants::temp_sz);
    Serializer<gps_time_t> gps_time_serializer;

    tank_inner_temperature =
        std::make_shared<ReadableStateField<signed int>>("prop.temp_inner", temperature_serializer);
    tank_outer_temperature =
        std::make_shared<ReadableStateField<signed int>>("prop.temp_outer", temperature_serializer);
    firing_time =
        std::make_shared<WritableStateField<gps_time_t>>("gnc.manuever.time", gps_time_serializer);

    registry.add_readable(tank_inner_temperature);
    registry.add_readable(tank_outer_temperature);
    registry.add_writable(firing_time);
}

void MainControlLoopTask::execute() { debug_task.execute(); }