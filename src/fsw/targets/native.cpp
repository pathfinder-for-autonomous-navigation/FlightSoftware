#include <fsw/FCCode/MainControlLoop.hpp>
#include <common/StateFieldRegistry.hpp>
#include "flow_data.hpp"
#include "eeprom_configs.hpp"

#ifndef UNIT_TEST
int main() {
    StateFieldRegistry registry;
    MainControlLoop fcp(registry, PAN::flow_data, PAN::statefields, PAN::periods);

    while (true) {
        fcp.execute();
    }
    return 0;
}
#endif
