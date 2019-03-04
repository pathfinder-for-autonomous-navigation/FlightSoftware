#include "../../state/state_holder.hpp"
#include "downlink_thread.hpp"

thread_t* Quake::downlink_thread;

THD_WORKING_AREA(Quake::downlink_thread_workingArea, 2048);

void Quake::go_to_waiting() {
    rwMtxWLock(&State::Quake::quake_state_lock);
        State::Quake::quake_state = State::Quake::QuakeState::WAITING;
    rwMtxWUnlock(&State::Quake::quake_state_lock);
}

THD_FUNCTION(Quake::downlink_fn, args) {
    Devices::QLocate::Message uplink;

    rwMtxRLock(&State::Quake::quake_state_lock);
        bool network_ready_interrupt_happened = State::Quake::network_ready_interrupt_happened;
    rwMtxRUnlock(&State::Quake::quake_state_lock);
    int response = 0;
    if (network_ready_interrupt_happened) {
        response = Quake::send_most_recent_downlink(&uplink);
        rwMtxWLock(&State::Quake::quake_state_lock);
            State::Quake::network_ready_interrupt_happened = false;
        rwMtxWUnlock(&State::Quake::quake_state_lock);
    }
    if (response == 0) response = Quake::send_downlink_stack(&uplink);
    if (uplink.get_length() != 0) {
        // Copy uplink into most recent uplink
    }
    go_to_waiting();
    chThdExit((msg_t) 0);
}