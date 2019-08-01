#include "master_helpers.hpp"

using namespace State::Master;

void Master::stop_safe_hold() {
  chThdTerminate(safe_hold_timer_thread);
  State::write(master_state, MasterState::DETUMBLE, master_state_lock);
  State::write(pan_state, PANState::MASTER_DETUMBLE, master_state_lock);
  // Here we set autoexit to false, since this function could be called by an
  // uplink packet
  State::write(autoexited_safe_hold, false, master_state_lock);

  chMtxLock(&eeprom_lock);
  EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_FLAG, false);
  EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_TIMER, (unsigned int)0);
  chMtxUnlock(&eeprom_lock);

  State::write(State::Propulsion::propulsion_state,
               State::Propulsion::PropulsionState::IDLE,
               State::Propulsion::propulsion_state_lock);
  dbg.println(debug_severity::INFO, "Safe hold completed!");
}

void Master::safe_hold() {
  dbg.println(debug_severity::NOTICE, "Entering safe hold mode...");
  State::write(master_state, MasterState::SAFE_HOLD, master_state_lock);
  State::write(pan_state, PANState::MASTER_SAFEHOLD, master_state_lock);

  // Disable ADCS
  State::write(State::ADCS::adcs_state, State::ADCS::ZERO_TORQUE,
               State::ADCS::adcs_state_lock);
  // Disable propulsion firings
  State::write(State::Propulsion::propulsion_state,
               State::Propulsion::PropulsionState::DISABLED,
               State::Propulsion::propulsion_state_lock);

  // Write to EEPROM
  chMtxLock(&eeprom_lock);
  EEPROM.put(EEPROM_ADDRESSES::FINAL_STATE_FLAG, (unsigned char)0);
  EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_FLAG, true);
  chMtxUnlock(&eeprom_lock);

  // Start safe hold timer
  safe_hold_timer_thread = chThdCreateStatic(
      &safe_hold_timer_workingArea, sizeof(safe_hold_timer_workingArea),
      RTOSTasks::master_thread_priority, safe_hold_timer, NULL);
}