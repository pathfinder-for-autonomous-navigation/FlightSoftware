#include <chrono>
#include <thread>
#include "FCCode/MainControlLoopTask.hpp"
#include <StateFieldRegistry.hpp>

#ifndef UNIT_TEST
int main(int argc, char *argv[]) {
    StateFieldRegistry registry;
    MainControlLoopTask fcp_task(registry);
    fcp_task.init();

    std::chrono::milliseconds sleep_duration(100);
    while (true) {
        fcp_task.execute();
        std::this_thread::sleep_for(sleep_duration);
    }
    return 0;
}
#endif
