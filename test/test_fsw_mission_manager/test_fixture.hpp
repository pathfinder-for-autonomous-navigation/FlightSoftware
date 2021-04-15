#ifndef TEST_FIXTURE_HPP_
#define TEST_FIXTURE_HPP_

#include "../StateFieldRegistryMock.hpp"

#include <fsw/FCCode/FaultHandlerMachine.hpp>
#include <fsw/FCCode/MissionManager.hpp>
#include <common/Fault.hpp>
#include <fsw/FCCode/mission_state_t.enum>
#include <fsw/FCCode/adcs_state_t.enum>
#include <fsw/FCCode/radio_state_t.enum>
#include <fsw/FCCode/prop_state_t.enum>
#include <fsw/FCCode/sat_designation_t.enum>

#include "../custom_assertions.hpp"
#include <lin/core.hpp>

class TestFixture
{
public:
  StateFieldRegistryMock registry;
  // Input state fields to mission manager
  std::shared_ptr<ReadableStateField<bool>> attitude_estimator_valid_fp;
  std::shared_ptr<ReadableStateField<lin::Vector3f>> attitude_estimator_L_body_fp;

  std::shared_ptr<WritableStateField<bool>> adcs_paired_fp;

  std::shared_ptr<ReadableStateField<unsigned char>> radio_state_fp;
  std::shared_ptr<ReadableStateField<unsigned int>> last_checkin_cycle_fp;

  std::shared_ptr<WritableStateField<unsigned int>> prop_state_fp;

  std::shared_ptr<ReadableStateField<unsigned char>> rel_orbit_state_fp;
  std::shared_ptr<ReadableStateField<lin::Vector3d>> rel_orbit_rel_pos_fp;

  std::shared_ptr<WritableStateField<bool>> reset_fp;
  std::shared_ptr<WritableStateField<bool>> power_cycle_radio_fp;

  std::shared_ptr<ReadableStateField<bool>> docked_fp;

  std::shared_ptr<Fault> low_batt_fault_fp;
  std::shared_ptr<Fault> adcs_functional_fault_fp;
  std::shared_ptr<Fault> wheel1_adc_fault_fp;
  std::shared_ptr<Fault> wheel2_adc_fault_fp;
  std::shared_ptr<Fault> wheel3_adc_fault_fp;
  std::shared_ptr<Fault> wheel_pot_fault_fp;
  std::shared_ptr<Fault> pressurize_fail_fp;
  std::shared_ptr<Fault> overpressured_fp;

  std::shared_ptr<WritableStateField<bool>> sph_dcdc_fp;
  std::shared_ptr<WritableStateField<bool>> adcs_dcdc_fp;

  std::shared_ptr<ReadableStateField<unsigned char>> piksi_state_fp;
  std::shared_ptr<InternalStateField<unsigned int>> last_rtkfix_ccno_fp;

  std::shared_ptr<ReadableStateField<unsigned int>> bootcount_fp;

  std::unique_ptr<MissionManager> mission_manager;

  // Output state fields from mission manager
  WritableStateField<double> *detumble_safety_factor_fp;
  WritableStateField<double> *close_approach_trigger_dist_fp;
  WritableStateField<double> *docking_trigger_dist_fp;
  WritableStateField<unsigned int> *docking_timeout_limit_fp;
  WritableStateField<unsigned char> *adcs_state_fp;
  WritableStateField<bool> *docking_config_cmd_fp;
  WritableStateField<unsigned char> *mission_state_fp;
  ReadableStateField<bool> *is_deployed_fp;
  ReadableStateField<unsigned int> *deployment_wait_elapsed_fp;
  WritableStateField<unsigned char> *sat_designation_fp;

  TestFixture(mission_state_t initial_state = mission_state_t::startup, unsigned int bootcount = 0);

  // Set and assert functions for various mission states.
  void set(fault_response_t response);
  void set(mission_state_t state);
  void set(adcs_state_t state);
  void set(prop_state_t state);
  void set(radio_state_t state);
  void set(sat_designation_t designation);
  void check(mission_state_t state) const;
  void check(adcs_state_t state) const;
  void check(prop_state_t state) const;
  void check(radio_state_t state) const;
  void check(sat_designation_t designation) const;

  // Check if the SpikeDoc DCDC pin is on or off.
  void check_sph_dcdc_on(bool on) const;
  void check_adcs_dcdc_on(bool on) const;

  // Containers for enum possibilities
  static adcs_state_t adcs_states[9];
  static prop_state_t prop_states[6];

  // Ensures that no state except the given state can be achieved.
  void assert_ground_uncommandability(adcs_state_t exception_state);
  void assert_ground_uncommandability(prop_state_t exception_mode);

  // Step forward the state machine by 1 control cycle.
  void step();

  //// Setter methods for setting test conditions.
  // Set the control cycle count
  void set_ccno(unsigned int ccno);
  // Set the distance between the two satellites.
  void set_sat_distance(double dist);
  // Set the # of control cycles that comms has not been established
  // with the ground.
  void set_comms_blackout_period(int ccno);
  // Set the angular rate of the spacecraft.
  void set_ang_rate(float rate);
  // Get the bootcount
  unsigned int get_bootcount();
};

#endif
