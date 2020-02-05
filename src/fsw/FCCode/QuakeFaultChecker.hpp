#ifndef QUAKE_FAULT_CHECKER_HPP_
#define QUAKE_FAULT_CHECKER_HPP_

#include "TimedControlTask.hpp"
#include "mission_state_t.enum"

/**
 * @brief This state machine class implements a tiered fault system
 * for addressing comms failures. See Issue #20 on Flight Software
 * for a description of what exactly it's implementing.
 */
class QuakeFaultChecker : public ControlTask<mission_state_t> {
  public:
    QuakeFaultChecker(StateFieldRegistry& r);

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
  
  protected:
    enum class fault_checker_state_t {
        unfaulted,
        forced_standby,
        powercycle_1,
        powercycle_2,
        powercycle_3,
        safehold
    };

    // Current state of fault checker DFA, and the control cycle
    // count at which it entered this state.
    fault_checker_state_t cur_state = fault_checker_state_t::unfaulted;
    unsigned int cur_state_entry_ccno = 0;

    mission_state_t dispatch_unfaulted();
    mission_state_t dispatch_forced_standby();
    mission_state_t dispatch_powercycle_1();
    mission_state_t dispatch_powercycle_2();
    mission_state_t dispatch_powercycle_3();
    mission_state_t dispatch_safehold();

    void transition_to(fault_checker_state_t next_state);

    const InternalStateField<unsigned char>* radio_state_fp;
    const InternalStateField<unsigned int>* last_checkin_cycle_fp;
    WritableStateField<bool>* power_cycle_radio_fp;

    const unsigned int& control_cycle_count = TimedControlTaskBase::control_cycle_count;
};

/**
 * @brief Mocking class that can be used by Mission Manager's unit test to
 * inject Quake faults in a controllable way.
 */
class QuakeFaultCheckerMock {
  public:
    QuakeFaultCheckerMock(StateFieldRegistry& r) {}

    mission_state_t execute() {
      return get();
    }
    mission_state_t get() const {
      return state;
    }
    void set(mission_state_t s) {
      state = s;
    }
  private:
    mission_state_t state = mission_state_t::manual;
};

#endif
