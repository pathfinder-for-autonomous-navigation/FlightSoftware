#include "../../state/state_holder.hpp"
#include "../constants.hpp"
#include "transceiving_thread.hpp"
#include "../../state/device_states.hpp"
#include "../../comms/uplink_deserializer.hpp"
#include <bitset>

using Devices::quake;
using State::Quake::quake_state_lock;
using State::Hardware::quake_device_lock;
using State::Quake::quake_state;
using State::Quake::QuakeState;
thread_t* Quake::transceiving_thread;

THD_WORKING_AREA(Quake::transceiving_thread_workingArea, 2048);

void Quake::go_to_waiting() {
    State::write(quake_state, QuakeState::WAITING, quake_state_lock);
}

static bool is_mt_queued() {
    chMtxLock(&State::Hardware::quake_device_lock);
        bool queued = quake->get_sbdix_response()[Devices::QLocate::MT_QUEUED];
    chMtxLock(&State::Hardware::quake_device_lock);
    return queued;
}

static void get_latest_uplink(QuakeMessage* uplink) {
    while(is_mt_queued() > 0) {
        int response = -1;
        if (!State::Hardware::check_is_functional(quake)) return;
        chMtxLock(&quake_device_lock);
            quake->run_sbdix();
        chMtxUnlock(&quake_device_lock);
        for(int i = 0; i < Constants::Quake::NUM_RETRIES; i++) {
            if (!State::Hardware::check_is_functional(quake)) return;
            chMtxLock(&quake_device_lock);
                response = quake->end_sbdix();
            chMtxUnlock(&quake_device_lock);
            
            if (response != -1) break;
            unsigned int wait_between_retries = Constants::read(Constants::Quake::WAIT_BETWEEN_RETRIES);
            chThdSleepMilliseconds(wait_between_retries);
        }
        if (response == 0) {
            State::write(State::Quake::sbdix_time_received, 
                State::GNC::get_current_time(), 
                State::Quake::quake_state_lock);
            
            if (!State::Hardware::check_is_functional(quake)) return;
            chMtxLock(&quake_device_lock);
                int status = quake->sbdrb();
            chMtxUnlock(&quake_device_lock);
            if (status == 0) *uplink = quake->get_message();
        }
    }
}

THD_FUNCTION(Quake::transceiving_fn, args) {
    QuakeMessage uplink;
    // Try sending as many downlinks as you can
    Quake::send_downlink_stack(&uplink);
    // Get the latest uplink that you can
    get_latest_uplink(&uplink);
    if (uplink.length != 0) {
        rwMtxWLock(&State::Quake::uplink_lock);
            std::bitset<Comms::UPLINK_SIZE_BITS> uplink_bitset(uplink.mes);
            Comms::deserialize_uplink(uplink_bitset, &State::Quake::most_recent_uplink);
        rwMtxWUnlock(&State::Quake::uplink_lock);
    }
    go_to_waiting();
    chThdExit((msg_t) 0);
}