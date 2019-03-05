#include "../../state/state_holder.hpp"
#include "../constants.hpp"
#include "downlink_thread.hpp"

using Devices::quake;
using Devices::QLocate;
using State::Quake::QuakeState;
using State::Quake::quake_state_lock;
using namespace Comms;

int send_packet(const QLocate::Message& packet, QLocate::Message* uplink) {
    int response;
    response = quake.sbdwb(packet.mes, State::Quake::PACKET_LENGTH);
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

int send_downlink(const State::Quake::full_data_downlink& downlink, QLocate::Message* uplink) {
    for(int i = 0; i < State::Quake::PACKETS_PER_DOWNLINK; i++) {
        rwMtxRLock(&quake_state_lock);
            QLocate::Message m(downlink[i]);
        rwMtxRUnlock(&quake_state_lock);
        int response = send_packet(m, uplink);
        if (response != 0) {
            // Sending packet failed. Throw this packet back on the stack and retry later.
            rwMtxWLock(&quake_state_lock);
                State::Quake::downlink_stack.put(downlink);
            rwMtxWUnlock(&quake_state_lock);
            return response;
        }
    }
    return 0;
}

bool is_downlink_stack_empty() {
    rwMtxRLock(&State::Quake::quake_state_lock);
        bool is_empty = State::Quake::downlink_stack.empty();
    rwMtxRLock(&State::Quake::quake_state_lock);
    return is_empty;
}

int Quake::send_downlink_stack(QLocate::Message* uplink) {
    while(!is_downlink_stack_empty()) {
        int response = send_downlink(State::Quake::downlink_stack.get(), uplink);
        if (response != 0) return response;
    }
    return 0;
}