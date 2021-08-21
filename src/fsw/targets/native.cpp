#include <fsw/FCCode/MainControlLoop.hpp>
#include <common/StateFieldRegistry.hpp>
#include "flow_data.hpp"

#include <csignal>

static volatile bool signal_handled = false;
static void signal_handler(int) {
    signal_handled = true;
}

#ifndef UNIT_TEST
int main() {
    std::signal(SIGTERM, signal_handler);

    StateFieldRegistry registry;
    MainControlLoop fcp(registry, PAN::flow_data);

    while (!signal_handled) {
        fcp.execute();
    }

    return 0;
}
#endif
