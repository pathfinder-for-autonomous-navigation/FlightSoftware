#include <fsw/FCCode/DummyMainControlLoop.hpp>
#include <common/StateFieldRegistry.hpp>

#ifndef UNIT_TEST
#include <csignal>

static volatile bool signal_handled = false;
static void signal_handler(int) {
    signal_handled = true;
}

int main() {
    std::signal(SIGTERM, signal_handler);

    StateFieldRegistry registry;
    DummyMainControlLoop fcp(registry);
    fcp.init();

    while (!signal_handled) {
        fcp.execute();
    }
    return 0;
}
#endif
