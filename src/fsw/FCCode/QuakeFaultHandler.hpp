#ifndef quake_fault_handler_HPP_
#define quake_fault_handler_HPP_

#include "TimedControlTask.hpp"
#include "mission_state_t.enum"

/**
 * @brief This state machine class implements a tiered fault system
 * for addressing comms failures. The fault system is as follows:
 * 
 * If Quake radio fails to initiate a successful SBDIX in 24 hours:
 * - Force the mission state to standby, wait 24 hours in that state.
 *   This puts the satellite in an comms-maximizing attitude.
 * - If we still don't have comms by then, power cycle the radio every 8 hours three times.
 * - If we still don't have comms by then, go into safe hold, which after 24 hours
 *   will reset the entire satellite.
 * 
 * NOTE: It's important that this machine does not override conditions
 * that have forced the satellite into safe hold, i.e. it should not
 * cause an autonomous transition from safe hold to standby. This important
 * caveat is managed by the MissionManager in its execute() function by
 * prioritizing transitions to safehold due to power, prop, and ADCS faults
 * over transitions to standby.
 * 
 * TODO unit test the shit out of this mission manager behavior.
 */
class QuakeFaultHandler : public ControlTask<mission_state_t> {
  public:
    #ifdef UNIT_TEST
    friend class TestFixture;
    #endif

    /**
     * @brief Construct a new Quake Fault Handler.
     * 
     * @param r State field registry. Needed so that the Quake fault
     * handler can check its indicator fields.
     */
    QuakeFaultHandler(StateFieldRegistry& r);

    /**
     * @brief Runs the fault state machine and returns the suggested
     * mission state.
     * 
     * @return mission_state_t Can either be standby, safe hold, or
     * manual. The mission manager should switch to standby or
     * safe hold if this state machine requires it, but should take
     * no action when manual is returned.
     */
    mission_state_t execute();

    /**
     * @brief States of fault machine. Description:
     * - Unfaulted: We've received comms in the last 24 hours
     * - Forced standby: We haven't had comms in the past 24 hours so the
     *   satellite is forced into standby mode to try to maximize comms.
     * - Power cycle 1: Forced standby didn't work. So the entry into this
     *   state causes a power cycle of the radio and waits 8 hours before
     *   transitioning to...
     * - Power cycle 2: Same behavior as power cycle 1.
     * - Power cycle 3: Same behavior as power cycle 2.
     * - Safe hold: Three power cycles didn't work, so we go to safe hold.
     *   The satellite waits in safe hold for 24 hours by default, after which
     *   the satellite is rebooted by high-level mission logic.
     */
    enum class fault_checker_state_t {
        unfaulted,
        forced_standby,
        powercycle_1,
        powercycle_2,
        powercycle_3,
        safehold
    };

  protected:

    /**
     * @brief Dispatch functions for the fault handler state machine.
     * 
     * @return mission_state_t 
     */
    mission_state_t dispatch_unfaulted();
    mission_state_t dispatch_forced_standby();
    mission_state_t dispatch_powercycle_1();
    mission_state_t dispatch_powercycle_2();
    mission_state_t dispatch_powercycle_3();
    mission_state_t dispatch_safehold();

    /**
     * @brief Executes a clean transition between fault states.
     * 
     * In the future, this should also trigger the recording of an event.
     * 
     * @param next_state Next state for transition.
     */
    void transition_to(fault_checker_state_t next_state);

    // Current state of fault checker DFA, and the control cycle
    // count at which it entered this state.
    fault_checker_state_t cur_state = fault_checker_state_t::unfaulted;
    unsigned int cur_state_entry_ccno = 0;

    /**
     * @brief Indicator fields used by the fault handler.
     */
    const InternalStateField<unsigned char>* radio_state_fp;
    const InternalStateField<unsigned int>* last_checkin_cycle_fp;
    WritableStateField<bool>* power_cycle_radio_fp;

    /**
     * @brief Helper functions for if-statements, to make them look cleaner.
     */
    bool less_than_one_day_since_successful_comms() const;
    bool in_state_for_more_than_time(const unsigned int time) const;
    bool radio_is_disabled() const;
};

/**
 * @brief Mocking class that can be used by Mission Manager's unit test to
 * have a simple way to emulate Quake fault behavior. It can use this class to
 * declaratively set the Quake fault handler's recommended mission state to
 * see if the mission manager responds appropriately.
 */
class QuakeFaultHandlerMock {
  public:
    QuakeFaultHandlerMock(StateFieldRegistry& r) {}

    mission_state_t execute() {
      return get_output();
    }
    mission_state_t get_output() const {
      return state;
    }
    void set_output(mission_state_t s) {
      const bool desired_state_is_valid = 
          (s == mission_state_t::standby) 
          || (s == mission_state_t::safehold)
          || (s == mission_state_t::manual);
      if(!desired_state_is_valid) assert(false);
      else state = s;
    }
  private:
    mission_state_t state = mission_state_t::manual;
};

#endif
