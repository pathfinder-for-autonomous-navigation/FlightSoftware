#include "controllers.hpp"
#include "../deployment_timer.hpp"
#include "../state/EEPROMAddresses.hpp"
#include "../state/fault_state_holder.hpp"
#include "../state/state_history_holder.hpp"
#include "../state/state_holder.hpp"
#include "constants.hpp"

void RTOSTasks::initialize_rtos_objects() {
  // Initialize all state locks
  rwMtxObjectInit(&State::Hardware::hardware_state_lock);
  rwMtxObjectInit(&State::Master::master_state_lock);
  rwMtxObjectInit(&State::ADCS::adcs_state_lock);
  rwMtxObjectInit(&State::Gomspace::gomspace_state_lock);
  rwMtxObjectInit(&State::Propulsion::propulsion_state_lock);
  rwMtxObjectInit(&State::GNC::gnc_state_lock);
  rwMtxObjectInit(&State::Piksi::piksi_state_lock);
  rwMtxObjectInit(&State::Quake::quake_state_lock);
  rwMtxObjectInit(&State::Quake::uplink_lock);
  rwMtxObjectInit(&Constants::changeable_constants_lock);
  rwMtxObjectInit(&RTOSTasks::LoopTimes::gnc_looptime_lock);
  rwMtxObjectInit(&FaultState::Propulsion::propulsion_faults_state_lock);
  rwMtxObjectInit(&FaultState::Gomspace::gomspace_faults_state_lock);
  rwMtxObjectInit(&FaultState::ADCS::adcs_faults_state_lock);
  // State history locks
  rwMtxObjectInit(&StateHistory::Propulsion::propulsion_history_state_lock);
  rwMtxObjectInit(&StateHistory::Piksi::piksi_history_state_lock);
  rwMtxObjectInit(&StateHistory::ADCS::adcs_history_state_lock);
  // Initialize all device locks
  chMtxObjectInit(&eeprom_lock);
  chMtxObjectInit(&State::Hardware::adcs_device_lock);
  chMtxObjectInit(&State::Hardware::dcdc_device_lock);
  chMtxObjectInit(&State::Hardware::spike_and_hold_device_lock);
  chMtxObjectInit(&State::Hardware::piksi_device_lock);
  chMtxObjectInit(&State::Hardware::system_output_device_lock);
  chMtxObjectInit(&State::Hardware::gomspace_device_lock);
  chMtxObjectInit(&State::Hardware::quake_device_lock);
  chMtxObjectInit(&State::Hardware::pressure_sensor_device_lock);
  chMtxObjectInit(&State::Hardware::temp_sensor_inner_device_lock);
  chMtxObjectInit(&State::Hardware::temp_sensor_outer_device_lock);
  chMtxObjectInit(&State::Hardware::docking_motor_device_lock);
  chMtxObjectInit(&State::Hardware::docking_switch_device_lock);
  // Initialize thread queues
  chThdQueueObjectInit(&adcs_detumbled);
  chThdQueueObjectInit(&deployment_timer_waiting);
}