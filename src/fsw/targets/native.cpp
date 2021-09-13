#include <fsw/FCCode/MainControlLoop.hpp>
#include <common/StateFieldRegistry.hpp>
#include "flow_data.hpp"
// #include "startup_flows.hpp"


#ifndef UNIT_TEST
int main() {
    StateFieldRegistry registry;
    MainControlLoop fcp(registry, PAN::flow_data, PAN::startup_flows, 
                        PAN::detumble_flows, PAN::close_approach_flows, 
                        PAN::docking_docked_flows);

    while (true) {
        fcp.execute();
    }
    return 0;
}
#endif
