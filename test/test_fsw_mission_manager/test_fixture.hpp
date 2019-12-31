#ifndef TEST_FIXTURE_HPP_
#define TEST_FIXTURE_HPP_

#include "../StateFieldRegistryMock.hpp"
#include "../src/FCCode/MissionManager.hpp"
#include "../src/FCCode/mission_state_t.enum"
#include "../src/FCCode/adcs_state_t.enum"
#include "../src/FCCode/radio_mode_t.enum"
#include "../src/FCCode/prop_mode_t.enum"
#include "../src/FCCode/sat_designation_t.enum"

#include <unity.h>
#include <lin.hpp>

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    // Input state fields to mission manager
    std::shared_ptr<InternalStateField<lin::Vector3f>> adcs_ang_momentum_fp;
    std::shared_ptr<WritableStateField<bool>> adcs_paired_fp;

    std::shared_ptr<InternalStateField<unsigned char>> radio_mode_fp;
    std::shared_ptr<InternalStateField<unsigned int>> last_checkin_cycle_fp;

    std::shared_ptr<ReadableStateField<unsigned char>> prop_mode_fp;

    std::shared_ptr<ReadableStateField<unsigned char>> piksi_mode_fp;
    std::shared_ptr<ReadableStateField<d_vector_t>> propagated_baseline_pos_fp;

    std::shared_ptr<ReadableStateField<bool>> docked_fp;

    std::unique_ptr<MissionManager> mission_manager;
    // Output state fields from mission manager
    WritableStateField<unsigned char>* adcs_state_fp;
    WritableStateField<bool>* docking_config_cmd_fp;
    WritableStateField<unsigned char>* mission_state_fp;
    ReadableStateField<bool>* is_deployed_fp;
    ReadableStateField<unsigned int>* deployment_wait_elapsed_fp;
    WritableStateField<unsigned char>* sat_designation_fp;

    TestFixture(mission_state_t initial_state = mission_state_t::startup);

    // Set and assert functions for various mission states.
    void set(mission_state_t state);
    void set(adcs_state_t state);
    void set(prop_mode_t mode);
    void set(radio_mode_t mode);
    void set(sat_designation_t designation);
    void check(mission_state_t state) const;
    void check(adcs_state_t state) const;
    void check(prop_mode_t mode) const;
    void check(radio_mode_t mode) const;
    void check(sat_designation_t designation) const;

    // Containers for enum possibilities
    static adcs_state_t adcs_states[8];
    static prop_mode_t prop_modes[2];

    // Ensures that no state except the given state can be achieved.
    void assert_ground_uncommandability(adcs_state_t exception_state);
    void assert_ground_uncommandability(prop_mode_t exception_mode);

    // Step forward the state machine by 1 control cycle.
    void step();

    //// Setter methods for setting test conditions.
    // Create a hardware fault that necessitates a transition to safe hold or initialization hold.
    void set_hardware_fault(bool faulted);
    // Set the distance between the two satellites.
    void set_sat_distance(double dist);
    // Set the # of control cycles that comms has not been established
    // with the ground.
    void set_comms_blackout_period(int ccno);
    // Set the angular rate of the spacecraft.
    void set_ang_rate(float rate);
};

#endif
