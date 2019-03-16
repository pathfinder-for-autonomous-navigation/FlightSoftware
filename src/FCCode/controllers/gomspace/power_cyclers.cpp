#include "power_cyclers.hpp"
#include "../../state/device_states.hpp"
#include "../../state/state_holder.hpp"

namespace Gomspace {
    thread_t* adcs_system_thread;
    thread_t* spike_and_hold_thread;
    thread_t* piksi_thread;
    thread_t* quake_thread;
}

MEMORYPOOL_DECL(Gomspace::power_cycler_pool, 1024, NULL);

THD_FUNCTION(Gomspace::cycler_fn, args) {
    cycler_arg_t* cycler_args = (cycler_arg_t*) args;
    
    chMtxLock(cycler_args->device_lock);
        Devices::gomspace().set_single_output(cycler_args->pin, 0);
        State::write(State::Hardware::hat.at(cycler_args->device).is_functional, 
            false, State::Hardware::hardware_state_lock);

        chThdSleepSeconds(30);
        Devices::gomspace().set_single_output(cycler_args->pin, 1);
        
        chThdSleepMilliseconds(10);
        State::write(State::Hardware::hat.at(cycler_args->device).is_functional, 
            (cycler_args->device)->is_functional(), State::Hardware::hardware_state_lock);
    chMtxUnlock(cycler_args->device_lock);
}