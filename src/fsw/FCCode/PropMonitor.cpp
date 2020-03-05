#include "PropMonitor.hpp"
#include "Drivers/PropulsionSystem.hpp"

PropMonitor::PropMonitor(StateFieldRegistry& r, unsigned int offset) :
    TimedControlTask<void>("prop_monitor_ct", r, offset)
    temp_tank1_f("prop.tank1.temp", Serializer<temperature_t>()),
    temp_tank2_f("prop.tank2.temp", Serializer<temperature_t>()),
    pressure_tank2_f("prop.tank2.pressure", Serializer<unsigned int>()),
{
    add_readable_field(temp_tank1_f);
    add_readable_field(temp_tank2_f);
    add_readable_field(pressure_tank2_f);

    temp_tank1_f.set(0);
    temp_tank2_f.set(0);
    pressure_tank2_f.set(0);
}

void PropMonitor::execute() {
    temp_tank1_f.set(Tank1.get_temp());
    temp_tank2_f.set(Tank2.get_temp());
    pressure_tank2_f.set(static_cast<unsigned int>(Tank2.get_pressure()));
}
