#ifndef POWER_CYCLERS_HPP_
#define POWER_CYCLERS_HPP_

#include <ChRt.h>
#include <Device.hpp>

namespace Gomspace {
struct cycler_arg_t {
    mutex_t *device_lock;
    Devices::Device *device;
    unsigned char pin;
};

extern memory_pool_t power_cycler_pool;
void cycler_fn(void *args);
extern thread_t *adcs_system_thread;
extern thread_t *spike_and_hold_thread;
extern thread_t *piksi_thread;
extern thread_t *quake_thread;
}  // namespace Gomspace

#endif