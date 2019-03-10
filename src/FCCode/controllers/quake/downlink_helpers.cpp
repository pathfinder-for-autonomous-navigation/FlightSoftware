#include "../../state/state_holder.hpp"
#include "../../comms/downlink_packet_generator.hpp"
#include "../constants.hpp"
#include "transceiving_thread.hpp"

using Devices::quake;
using Devices::QLocate;
using State::Quake::QuakeState;
using State::Quake::quake_state_lock;
using namespace Comms;

static int send_packet(const QLocate::Message& packet, QLocate::Message* uplink) {
    int response;
    response = quake.sbdwb(packet.mes, Comms::PACKET_SIZE_BYTES);
    if (response != 0) return response;
    
    quake.run_sbdix();
    for(int i = 0; i < Constants::Quake::NUM_RETRIES; i++) {
        response = quake.end_sbdix();
        if (response != -1) break;
        chThdSleepMilliseconds(Constants::Quake::WAIT_BETWEEN_RETRIES);
    }

    // It's possible we picked up an uplink packet; pick it up.
    int status = quake.sbdrb();
    if (status == 0) *uplink = quake.get_message();

    return response;
}

static bool is_downlink_stack_empty() {
    bool is_empty = State::read(State::Quake::downlink_stack.empty(), State::Quake::quake_state_lock);
    return is_empty;
}

int Quake::send_downlink_stack(QLocate::Message* uplink) {
    while(!is_downlink_stack_empty()) {
        int response = send_packet(State::Quake::downlink_stack.get(), uplink);
        if (response != 0) return response;
    }
    return 0;
}