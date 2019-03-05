#ifndef DOWNLINK_THREAD_HPP_
#define DOWNLINK_THREAD_HPP_

#include "../controllers.hpp"
#include <QLocate/QLocate.hpp>

namespace Quake {
    extern THD_WORKING_AREA(downlink_thread_workingArea, 2048);
    extern THD_FUNCTION(downlink_fn, args);
    extern thread_t* downlink_thread;

    int send_downlink_stack(Devices::QLocate::Message* uplink);

    void go_to_waiting();

    constexpr tprio_t downlink_thread_priority = RTOSTasks::quake_thread_priority;
}

#endif