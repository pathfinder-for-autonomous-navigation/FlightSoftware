/** @file quake.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for the Quake state controller.
 */

#include "controllers.hpp"
#include "../state/state_holder.hpp"
#include "../deployment_timer.hpp"
#include "../comms/downlink_serializer.hpp"
#include "../comms/uplink_deserializer.hpp"
#include <HardwareSerial.h>

namespace RTOSTasks {
    THD_WORKING_AREA(quake_controller_workingArea, 16384);
}
using Devices::quake;
using Devices::QLocate;
using namespace State::Quake;
using namespace Comms;

static std::array<char, PACKET_SIZE_BYTES> downlink_packets[3]; // There are three downlink packets
static std::array<char, PACKET_SIZE_BYTES> uplink_packet; // The location for the packed uplink packet

unsigned int receive_uplink() {
    int status = quake.sbdrb();
    if (status != 0) return status;
    memcpy(quake.get_message().mes, uplink_packet.data(), PACKET_SIZE_BYTES);
    return status;
}

unsigned int send_packet(int packet_number) {
    debug_printf("Sending packet %d\n", packet_number);

    int response = -1;
    quake.sbdwb(downlink_packets[packet_number-1].data(), PACKET_SIZE_BYTES);
    quake.run_sbdix();
    for(int i = 0; i < 5; i++) {
        response = quake.end_sbdix();
        if (response != -1) break;
        chThdSleepMilliseconds(RTOSTasks::WAIT_BETWEEN_RETRIES);
    }
    if (response == 0) {
        // OK; packet submission is done; wait for uplink!
        debug_printf("Succeeded in sending downlink packet %d.\n", packet_number);
        return 1;
    }
    else {
        // Could not send packet; don't try sending it again.
        debug_printf("Failed in in sending downlink packet %d.\n", packet_number);
        return 0;
    }
}

unsigned int send_downlink() {
    if (send_packet(1) == 0) {
        int packet2 = send_packet(2);
        int packet3 = send_packet(3);
        return 1 + packet2 + packet3;
    }
    else return 0;
}

static void quake_loop(systime_t deadline) {
    debug_println("Serializing downlink");
    Comms::downlink_serializer(downlink_packets);
    debug_println("Trying to send downlink");
    systime_t send_attempt_start = chVTGetSystemTimeX();
    systime_t send_attempt_end = deadline - MS2ST(RTOSTasks::TRY_DOWNLINK_UNTIL);
    while(chVTIsSystemTimeWithin(send_attempt_start, send_attempt_end)) {
        if (send_downlink() > 0) break;
        chThdSleepMilliseconds(RTOSTasks::WAIT_BETWEEN_RETRIES);
    }

    chThdSleepUntil(send_attempt_end);

    debug_println("Trying to receive uplink");
    systime_t receive_attempt_start = chVTGetSystemTimeX();
    systime_t receive_attempt_end = deadline - MS2ST(RTOSTasks::TRY_UPLINK_UNTIL);
    unsigned int receive_status = -1;
    while(chVTIsSystemTimeWithin(receive_attempt_start, receive_attempt_end)) {
        receive_status = receive_uplink();
        if (receive_status == 0) break;
        chThdSleepMilliseconds(RTOSTasks::WAIT_BETWEEN_RETRIES);
    }
    if(receive_status == 0) {
        debug_println("Succeeded in retrieving uplink");
        rwMtxWLock(&quake_state_lock);
            State::Quake::missed_uplinks = 0;
        rwMtxWUnlock(&quake_state_lock);
        rwMtxWLock(&uplink_lock);
            Comms::uplink_deserializer(uplink_packet, &most_recent_uplink);
        rwMtxWUnlock(&uplink_lock);
    }
    else {
        debug_println("Failed to receive uplink");
        rwMtxWLock(&quake_state_lock);
            State::Quake::missed_uplinks++;
        rwMtxWUnlock(&quake_state_lock);
    }
}

void RTOSTasks::quake_controller(void *arg) {
    chRegSetThreadName("QUAKE");
    debug_println("Quake radio controller process has started.");
    debug_println("Waiting for deployment timer to finish.");
    rwMtxRLock(&State::Master::master_state_lock);
        bool is_deployed = State::Master::is_deployed;
    rwMtxRUnlock(&State::Master::master_state_lock);
    if (!is_deployed) chThdEnqueueTimeoutS(&deployment_timer_waiting, S2ST(DEPLOYMENT_LENGTH));
    debug_println("Deployment timer has finished.");
    debug_println("Initializing main operation...");

    systime_t deadline = chVTGetSystemTimeX();
    while(true) {
        deadline += MS2ST(RTOSTasks::LoopTimes::QUAKE); // TODO move to constant
        quake_loop(deadline);
        chThdSleepUntil(deadline);
    }

    chThdExit((msg_t)0);
}