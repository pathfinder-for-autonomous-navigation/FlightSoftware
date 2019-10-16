#include "MainControlLoopTask.hpp"
#include "DebugTask.hpp"

MainControlLoopTask::MainControlLoopTask(StateFieldRegistry& registry)
    : ControlTask<void>("fcp", registry), debug_task(registry) {
    Serializer<signed int> temperature_serializer(-40, 125, Serializer<signed int>::temp_sz);
    Serializer<gps_time_t> gps_time_serializer;

    tank_inner_temperature =
        std::make_shared<ReadableStateField<signed int>>("prop.temp_inner", temperature_serializer);
    tank_outer_temperature =
        std::make_shared<WritableStateField<signed int>>("prop.temp_outer", temperature_serializer);

    registry.add_readable_field(tank_inner_temperature);
    registry.add_readable_field(tank_outer_temperature);
}

void MainControlLoopTask::execute() {
    debug_task.execute();
}
