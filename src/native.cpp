#include "FCCode/MainControlLoopTask.hpp"
#include <StateFieldRegistry.hpp>

#ifndef UNIT_TEST
int main(int argc, char *argv[]) {
    StateFieldRegistry registry;
    MainControlLoopTask fcp_task(registry);
    fcp_task.init();

    while (true) {
        fcp_task.execute();
    }
    return 0;
}
#endif
