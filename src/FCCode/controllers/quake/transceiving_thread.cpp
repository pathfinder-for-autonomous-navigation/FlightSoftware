#include "../../state/state_holder.hpp"
#include "../constants.hpp"
#include "transceiving_thread.hpp"
#include "../../state/device_states.hpp"
#include "../../comms/uplink_deserializer.hpp"
#include <bitset>

thread_t* Quake::transceiving_thread;

THD_WORKING_AREA(Quake::transceiving_thread_workingArea, 2048);

void Quake::go_to_waiting() {
    State::write(State::Quake::quake_state, State::Quake::QuakeState::WAITING, State::Quake::quake_state_lock);
}

static void get_latest_uplink(Devices::QLocate::Message* uplink) {
    while(Devices::quake.get_sbdix_response()[Devices::QLocate::MT_QUEUED] > 0) {
        int response = -1;
        if (!State::Hardware::can_get_data(Devices::quake)) return;
        chMtxLock(&State::Hardware::quake_device_lock);
            Devices::quake.run_sbdix();
        chMtxUnlock(&State::Hardware::quake_device_lock);
        for(int i = 0; i < Constants::Quake::NUM_RETRIES; i++) {
            chMtxLock(&State::Hardware::quake_device_lock);
                response = Devices::quake.end_sbdix();
            chMtxUnlock(&State::Hardware::quake_device_lock);
            if (response != -1) break;
            unsigned int wait_between_retries = Constants::read(Constants::Quake::WAIT_BETWEEN_RETRIES);
            chThdSleepMilliseconds(wait_between_retries);
        }
        if (response == 0) {
            chMtxLock(&State::Hardware::quake_device_lock);
                int status = Devices::quake.sbdrb();
            chMtxUnlock(&State::Hardware::quake_device_lock);
            if (status == 0) *uplink = Devices::quake.get_message();
        }
    }
}

THD_FUNCTION(Quake::transceiving_fn, args) {
    Devices::QLocate::Message uplink;
    // Try sending as many downlinks as you can
    Quake::send_downlink_stack(&uplink);
    // Get the latest uplink that you can
    get_latest_uplink(&uplink);
    if (uplink.get_length() != 0) {
        rwMtxWLock(&State::Quake::uplink_lock);
            std::bitset<Comms::UPLINK_SIZE_BITS> uplink_bitset(uplink.mes);
            Comms::deserialize_uplink(uplink_bitset, &State::Quake::most_recent_uplink);
        rwMtxWUnlock(&State::Quake::uplink_lock);
    }
    go_to_waiting();
    chThdExit((msg_t) 0);
}