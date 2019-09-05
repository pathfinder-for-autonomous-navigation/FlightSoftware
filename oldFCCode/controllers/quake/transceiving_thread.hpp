#ifndef DOWNLINK_THREAD_HPP_
#define DOWNLINK_THREAD_HPP_

#include <QLocate.hpp>
#include "../controllers.hpp"

namespace Quake {
extern THD_WORKING_AREA(transceiving_thread_workingArea, 2048);
extern THD_FUNCTION(transceiving_fn, args);
extern thread_t *transceiving_thread;

int send_downlink_stack(QuakeMessage *uplink);

void go_to_waiting();

constexpr tprio_t transceiving_thread_priority = RTOSTasks::quake_thread_priority;
}  // namespace Quake

#endif