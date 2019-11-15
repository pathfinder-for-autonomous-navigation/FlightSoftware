#include "FCCode/MainControlLoop.hpp"
#include <StateFieldRegistry.hpp>

#ifndef UNIT_TEST
int main() {
    StateFieldRegistry registry;
    MainControlLoop fcp(registry);
    fcp.init();

    while (true) {
        fcp.execute();
    }
    return 0;
}
#endif
