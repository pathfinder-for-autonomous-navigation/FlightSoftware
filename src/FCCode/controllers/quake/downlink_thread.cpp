#include "../../state/state_holder.hpp"
#include "downlink_thread.hpp"
#include "../../comms/uplink_deserializer.hpp"

thread_t* Quake::downlink_thread;

THD_WORKING_AREA(Quake::downlink_thread_workingArea, 2048);

void Quake::go_to_waiting() {
    State::write(State::Quake::quake_state, State::Quake::QuakeState::WAITING, State::Quake::quake_state_lock);
}

THD_FUNCTION(Quake::downlink_fn, args) {
    Devices::QLocate::Message uplink;
    Quake::send_downlink_stack(&uplink);
    if (uplink.get_length() != 0) {
        rwMtxWLock(&State::Quake::uplink_lock);
            Comms::uplink_deserializer(uplink, &State::Quake::most_recent_uplink);
        rwMtxWUnlock(&State::Quake::uplink_lock);
    }
    go_to_waiting();
    chThdExit((msg_t) 0);
}