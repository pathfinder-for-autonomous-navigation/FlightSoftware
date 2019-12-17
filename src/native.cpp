#include "FCCode/MainControlLoop.hpp"
#include <StateFieldRegistry.hpp>
#include "flow_data.hpp"

#ifndef UNIT_TEST
int main() {
    StateFieldRegistry registry;
    MainControlLoop fcp(registry, PAN::flow_data);

    while (true) {
        fcp.execute();
    }
    return 0;
}
#endif
