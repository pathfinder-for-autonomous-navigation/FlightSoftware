#include "../../state/state_holder.hpp"
#include "../../comms/downlink_serializer.hpp"
#include "../constants.hpp"
#include "transceiving_thread.hpp"

using Devices::quake;
using State::Quake::QuakeState;
using State::Quake::quake_state_lock;
using namespace Comms;

static int send_packet(const QuakeMessage& packet, QuakeMessage* uplink) {
    int response;
    if (State::Hardware::check_is_functional(&quake())) {
        chMtxLock(&State::Hardware::quake_device_lock);
            response = quake().sbdwb(packet.mes, Comms::PACKET_SIZE_BYTES);
        chMtxUnlock(&State::Hardware::quake_device_lock);
    }
    else return -1;
    if (response != 0) return response;
    
    if (State::Hardware::check_is_functional(&quake())) {
        chMtxLock(&State::Hardware::quake_device_lock);
            quake().run_sbdix();
        chMtxUnlock(&State::Hardware::quake_device_lock);
    }
    else return -1;

    for(int i = 0; i < Constants::read(Constants::Quake::NUM_RETRIES); i++) {
        chMtxLock(&State::Hardware::quake_device_lock);
            response = quake().end_sbdix();
        chMtxUnlock(&State::Hardware::quake_device_lock);
        if (response != -1) break;
        unsigned int wait_between_retries = Constants::read(Constants::Quake::WAIT_BETWEEN_RETRIES);
        chThdSleepMilliseconds(wait_between_retries);
    }

    // It's possible we picked up an uplink packet; pick it up.
    if (State::Hardware::check_is_functional(&quake())) {
        chMtxLock(&State::Hardware::quake_device_lock);
            int status = quake().sbdrb();
            if (status == 0) *uplink = quake().get_message();
        chMtxUnlock(&State::Hardware::quake_device_lock);
    }
    else return -1;

    if (response == 0) State::write(State::Quake::sbdix_time_received, 
                                    State::GNC::get_current_time(), 
                                    State::Quake::quake_state_lock);
    return response;
}

static bool is_downlink_stack_empty() {
    bool is_empty = State::read(State::Quake::downlink_stack.empty(), State::Quake::quake_state_lock);
    return is_empty;
}

int Quake::send_downlink_stack(QuakeMessage* uplink) {
    while(!is_downlink_stack_empty()) {
        int response = send_packet(State::Quake::downlink_stack.get(), uplink);
        if (response != 0) return response;
        // TODO requeue packet on stack if response is nonzero
    }
    return 0;
}