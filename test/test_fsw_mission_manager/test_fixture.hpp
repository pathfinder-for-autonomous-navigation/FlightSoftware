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

#include <unity.h>
#include <lin.hpp>

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    // Input state fields to mission manager
    std::shared_ptr<InternalStateField<lin::Vector3f>> adcs_ang_momentum_fp;
    std::shared_ptr<WritableStateField<bool>> adcs_paired_fp;

    std::shared_ptr<InternalStateField<unsigned char>> radio_state_fp;
    std::shared_ptr<InternalStateField<unsigned int>> last_checkin_cycle_fp;

    std::shared_ptr<ReadableStateField<unsigned char>> prop_state_fp;

    std::shared_ptr<ReadableStateField<lin::Vector3d>> propagated_baseline_pos_fp;

    std::shared_ptr<WritableStateField<bool>> reboot_fp;
    std::shared_ptr<WritableStateField<bool>> power_cycle_radio_fp;

    std::shared_ptr<ReadableStateField<bool>> docked_fp;
    //std::shared_ptr<InternalStateField<unsigned int>> enter_docking_cycle_fp;

    std::shared_ptr<Fault> low_batt_fault_fp;
    std::shared_ptr<Fault> adcs_functional_fault_fp;
    std::shared_ptr<Fault> wheel1_adc_fault_fp;
    std::shared_ptr<Fault> wheel2_adc_fault_fp;
    std::shared_ptr<Fault> wheel3_adc_fault_fp;
    std::shared_ptr<Fault> wheel_pot_fault_fp;
    std::shared_ptr<Fault> failed_pressurize_fp;
    std::shared_ptr<Fault> overpressured_fp;

    std::unique_ptr<MissionManager> mission_manager;

    // Output state fields from mission manager
    WritableStateField<double>* detumble_safety_factor_fp;
    WritableStateField<double>* close_approach_trigger_dist_fp;
    WritableStateField<double>* docking_trigger_dist_fp;
    WritableStateField<unsigned int>* max_radio_silence_duration_fp;
    WritableStateField<unsigned int>* docking_timeout_limit_fp;
    WritableStateField<unsigned char>* adcs_state_fp;
    WritableStateField<bool>* docking_config_cmd_fp;
    WritableStateField<unsigned char>* mission_state_fp;
    ReadableStateField<bool>* is_deployed_fp;
    ReadableStateField<unsigned int>* deployment_wait_elapsed_fp;
    WritableStateField<unsigned char>* sat_designation_fp;

    TestFixture(mission_state_t initial_state = mission_state_t::startup);

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

    // Containers for enum possibilities
    static adcs_state_t adcs_states[8];
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
};

#endif
