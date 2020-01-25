#include <fsw/FCCode/DummyMainControlLoop.hpp>
#include <common/StateFieldRegistry.hpp>

#ifndef UNIT_TEST
int main() {
    StateFieldRegistry registry;
    DummyMainControlLoop fcp(registry);
    fcp.init();

    while (true) {
        fcp.execute();
    }
    return 0;
}
#endif
