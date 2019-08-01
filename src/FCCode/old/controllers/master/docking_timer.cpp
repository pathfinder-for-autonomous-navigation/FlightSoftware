#include "master_helpers.hpp"

virtual_timer_t Master::docking_timer;

void Master::stop_docking_mode(void *args) {
  State::write(State::Master::pan_state, State::Master::PANState::STANDBY,
               State::Master::master_state_lock);
}